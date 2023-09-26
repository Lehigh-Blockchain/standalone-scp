#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <bits/stdc++.h>
#include <tuple>

// Stellar includes
#include <crypto/Hex.h>
#include <crypto/SecretKey.h>
#include <crypto/SHA.h>
#include <scp/SCP.h>
#include <scp/Slot.h>
#include <scp/QuorumSetUtils.h>
#include <util/Logging.h>
#include <util/XDROperators.h>
#include <lib/json/json.h>

using namespace std;

/* format for node-input.txt

NODE_NAME
NODE_IN_QUORUM_SLICE,NODE_IN_QUORUM_SLICE,NODE_IN_QUORUM_SLICE

NODE_NAME
NODE_IN_QUORUM_SLICE,NODE_IN_QUORUM_SLICE,NODE_IN_QUORUM_SLICE

NODE_NAME
NODE_IN_QUORUM_SLICE,NODE_IN_QUORUM_SLICE,NODE_IN_QUORUM_SLICE

etc.
First line is the name of the node (turned into a NodeID type), second line is the names of the 
nodes in it's quorum slice (turned into a SCPQuorumSet type), and the third line is blank.
*/

/*
    GENERAL TODOs
        [] when accessing a mapping like "curr_node = name_to_node[node_name];" if the key node_name doesn't
            yet exist in the mapping name_to_node it will create a new entry to the mapping.  We don't really
            want this so maybe throw an error if this happens.
        [] mess around with node-input.txt until you find some unexpected behavior.  Look at the printed output,
            specifically with the "printing quorum slices" section because that is where the parser is weakest.
        [] (later) If we eventually find the need to, modify the parser to support inner sets.  This would 
            be a large change because we would have to change the format of node-input.txt
*/

stellar::SCPQuorumSetPtr initQSet(xdr::xvector<stellar::NodeID> const& nodes) {
    xdr::xvector<stellar::SCPQuorumSet> innerSets;
    auto qset = std::make_shared<stellar::SCPQuorumSet>((nodes.size())/2 + 1, nodes, innerSets);
    normalizeQSet(*qset);
    return qset;
}

tuple<
    xdr::xvector<stellar::NodeID>,
    map<stellar::NodeID, stellar::SCPQuorumSetPtr>,
    map<stellar::NodeID, string>> 
parseInput(string filename) {

    ifstream input_file;
    input_file.open(filename);

    xdr::xvector<stellar::NodeID> node_vec;
    map<stellar::NodeID, stellar::SCPQuorumSetPtr> node_to_quorum;
    map<stellar::NodeID, string> node_to_name;
    map<stellar::NodeID, vector<string>> node_to_quorum_names;
    map<string, stellar::NodeID> name_to_node;

    string node_name;
    string quorum_str;
    stellar::NodeID new_node;

    while(getline(input_file, node_name)){
        new_node = stellar::PubKeyUtils::random(); 
        node_vec.push_back(new_node);
        node_to_name[new_node] = node_name;
        name_to_node[node_name] = new_node;

        if(getline(input_file, quorum_str)) {
            stringstream ss(quorum_str);
            string token;
            vector<string> quorum_names;
            while (getline(ss, token, ',')) {
                token.erase(0, token.find_first_not_of(' '));
                token.erase(token.find_last_not_of(' ') + 1);
                quorum_names.push_back(token);
            }
            node_to_quorum_names[new_node] = quorum_names;
        }

        getline(input_file, node_name); // Read the blank line
    }

    for (stellar::NodeID i : node_vec) {
        vector<string> quorum_names = node_to_quorum_names.at(i);
        xdr::xvector<stellar::NodeID> quorum_vec;
        quorum_vec.push_back(i); // Add the node itself to its quorum slice
        for (const string& node_name : quorum_names) {
            stellar::NodeID curr_node = name_to_node[node_name];
            quorum_vec.push_back(curr_node);
        }
        node_to_quorum[i] = initQSet(quorum_vec);
    }

    input_file.close();

    return make_tuple(node_vec, node_to_quorum, node_to_name);
}
