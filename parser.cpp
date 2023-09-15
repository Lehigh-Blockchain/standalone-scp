#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <bits/stdc++.h>
#include <tuple>
#include <sstream>

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
NODE_IN_QUORUM_SLICE NODE_IN_QUORUM_SLICE NODE_IN_QUORUM_SLICE

NODE_NAME
NODE_IN_QUORUM_SLICE NODE_IN_QUORUM_SLICE NODE_IN_QUORUM_SLICE

NODE_NAME
NODE_IN_QUORUM_SLICE NODE_IN_QUORUM_SLICE NODE_IN_QUORUM_SLICE

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

    TODOs and issues for this file
        [] Does not do anything if there is two of the same country in a quorum slice (Yiannis: usa usa)
        [] Does not do anything if there are two of the same nodes with different quorem slices (does a weird thing
        where it still prints out just not the quorem slice)
        [] issue with adding more than 2 contries in a quorem slice (Prints blank only at the end, if you add to middle of slice it works)
        [] does not understand when the next node starts if there is no space, lets account for that
        [] issues with two word contries like 'New Zealand'
        [] when you add into quorum slice it thinks some countries are its own node (Big Issue)

*/

// From reference code executable.cpp, but it's being used differently
// in executable.cpp, this is called once to create a single quorum set for a network
// in our model, we want to support many quorum sets for a network, so we call this for each node
stellar::SCPQuorumSetPtr initQSet(xdr::xvector<stellar::NodeID> const& nodes) {
    // TODO: support inner sets (we don't need to do this yet so maybe wait)
    xdr::xvector<stellar::SCPQuorumSet> innerSets;
    // first argument: threshold of nodes needed to agree to a statement
    // second argument: vector of nodeIDs that are in the nodes's quorum slice
    // third argument: inner set is a nested quorum set
    auto qset = std::make_shared<stellar::SCPQuorumSet>((nodes.size())/2 + 1, nodes, innerSets);
    normalizeQSet(*qset);
    return qset;
}

// reads input file of nodes and their slices into a nodeID vec
// returns a tuple of nodeID vector, nodeID->quorum mapping, and nodeID->name mapping
tuple<
    xdr::xvector<stellar::NodeID>,
    map<stellar::NodeID, stellar::SCPQuorumSetPtr>,
    map<stellar::NodeID, string>> 
parseInput(string filename) {

    // open file
    ifstream input_file;
    input_file.open(filename);

    // return value: vector of nodeIDs
    xdr::xvector<stellar::NodeID> node_vec;
    // return value: nodeID->quorum mapping
    map<stellar::NodeID, stellar::SCPQuorumSetPtr> node_to_quorum;
    // return value: nodeID->name mapping
    map<stellar::NodeID, string> node_to_name;

    // mapping to hold the nodeID to the names of the nodes in it's quorum slice
    map<stellar::NodeID, string> node_to_quorum_names;

    // mapping to associate the name of a node to it's nodeID
    // Used when constructing quorum slice of a node
    map<string, stellar::NodeID> name_to_node;

    string node_name;
    string quorum_str;
    stellar::NodeID new_node;

    // run until End Of File is reached
    while(!input_file.eof()){

        // get name
        getline(input_file, node_name);
        // Generate new nodeID by creating random key
        // NodeID type is just a public key
        new_node = stellar::PubKeyUtils::random(); 

        // Add new nodeID to the returned vector of nodeIDs
        node_vec.push_back(new_node);

        // Add new nodeID to the returned map nodeID->name
        node_to_name[new_node] = node_name;

        // Add new node to the name->nodeID map
        name_to_node[node_name] = new_node;

        // get quorum slice as string to be later converted to a qSet
        getline(input_file, quorum_str);
        node_to_quorum_names[new_node] = quorum_str;

        // get a blank line
        // doesn't matter which string buffer is used because it'll get overwritten
        getline(input_file, node_name);
    }

    // loop through nodeID vector and construct qSet for each node
    // This has to be done after to make sure we discovered all the nodes in node-input.txt
    xdr::xvector<stellar::NodeID> quorum_nodes;
    string quorum_nodes_str;
    for (stellar::NodeID i : node_vec) {
        // TODO: handle case when a nodeID isn't in this map
        // String of node names
        quorum_nodes_str = node_to_quorum_names.at(i);
        // vector of nodeIDs

        xdr::xvector<stellar::NodeID> quorum_vec;
        // TODO: not sure if I need to add the node whos owns this slice to the vector, or if it is done in TestSCP initialization
        quorum_vec.push_back(i);
        stringstream ss(quorum_nodes_str);
        string node_name;
        stellar::NodeID curr_node;
        // turns node name strings (space separated) into a vector of NodeID
        while (ss >> node_name) {
            // gets the nodeID of the current node name
            // TODO: handle case when node_name doesn't yet exist in mapping
            curr_node = name_to_node[node_name];
            // Add the nodeID to the vector
            quorum_vec.push_back(curr_node);
        }

        // map of nodeID to Qset
        node_to_quorum[i] = initQSet(quorum_vec);
    }

    // close file
    input_file.close();

    // return tuple
    return make_tuple(node_vec, node_to_quorum, node_to_name);
}
