#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <bits/stdc++.h>
#include <tuple>
#include "main.h"
#include "parser.h"

#define USE_SPDLOG // ??

using namespace std;

/*
    GENERAL TODO:
        [] change pointers to smart/unique pointers where appropriate 
        [] move pasrsing functions into their own file.  This will require some makefile refactoring
        [] using namespace std here because I figured it would be better to be explicit about which types are from stellar
            this might not be very readable though, and also confusing since main.h uses namespace stellar.  Resolve this
            so main.cpp and main.h use the same namespace
        [] Currently doesn't support inner sets.  node-input parsing will have to be re-worked to support inner sets.
            Inner sets as whole networks might be the key to hierarchical consensus.
        [] (maybe not needed) Support a custom threshold value of node-input.txt.  Currently it is majority.
        [] Make parsing more robust.  Lotta issues.  play around with node-input until it breaks
        [] in the logging, qSets seem to be the same.  Potentially an issue.  All qSets print as "030000"
*/


// Each node will have an instance of this class which contains the SCP state machine
// and also is an SCPDriver which mediates communication between Ivy and that SCP.
// class TestSCP : public SCPDriver {...}
/*
void stellar::TestSCP::signEnvelope(SCPEnvelope& envelope)
{
    // Do nothing -- we don't bother with signatures in this model.
}
*/

/*
stellar::SCPQuorumSetPtr stellar::TestSCP::getQSet(Hash const& qSetHash)
{
    // We support exactly 1 qset in this model. (TODO: are we sure about this?)
    // TODO: How does it access gNetwork when it's uninitialized
    if (qSetHash != gNetwork->mQSetHash)
    {
        setLocalLogPrefix();
        cout<< "can't find qset hash {}" + hexAbbrev(qSetHash);
        throw std::runtime_error("can't find qset hash");
    }
    return gNetwork->mQSet;
}
*/

// create a vector of Hashes from all of the SCPQuorumSetPtrs (do I need to dereference SCPQuorumSetPtr)
const map<stellar::SCPQuorumSetPtr, stellar::Hash> computeQSetHash(const std::map<stellar::NodeID, stellar::SCPQuorumSetPtr> node_to_quorum) {
    map<stellar::SCPQuorumSetPtr, stellar::Hash> hashes;
    stellar::Hash curr_hash;
    for (const auto& [curr_NodeID, curr_SCPQuorumSetPtr] : node_to_quorum)
    {
        curr_hash = stellar::xdrSha256(*curr_SCPQuorumSetPtr);
        hashes[curr_SCPQuorumSetPtr] = curr_hash;
    }

    return hashes;
}

// Network constructor
// In reference code, constructor just takes an integer and creates 
// that many nodes as well as a single quorum set for those node.
// In our model, we have a dynamic amount of nodes and quorum sets
stellar::Network::Network(
    const xdr::xvector<stellar::NodeID> *node_vec, 
    const map<stellar::NodeID, stellar::SCPQuorumSetPtr> *node_to_quorum
)
    :mNodeIDs(*node_vec),
    mQSet(*node_to_quorum),
    // TODO: function to make a hash of a map
    // This will have to be an array or map for us, since reference-code model deals with only one qSet and ours
    // handles any amount of qSets
    mQSetHash(computeQSetHash(mQSet))
{}

// TestSCP constructor
stellar::TestSCP::TestSCP(NodeID const& nodeID, SCPQuorumSet const& qSetLocal)
    // third arg is if node is validator or not
    // TODO: should all nodes be validators?
    : mSCP(*this, nodeID, true, qSetLocal) {};

void stellar::TestSCP::setLocalLogPrefix() {
    Logging::setFmt(KeyUtils::toShortString(mSCP.getLocalNodeID()));
}


void stellar::TestSCP::signEnvelope(SCPEnvelope& envelope) {
    // reference-code doesn't bother with implementing signatures
}


// takes qSetHash and returns the associated qSet
// TODO: our implementation of getQSet will need to iterate over all the qSets and compare all hashes
// TODO: might be cleaner to pass in our gNetwork as an argument, but not sure if changing the signature would make
// it incompatible with stellar-core
// Initial attempt seems pretty hacky.  Iterating through a mapping (key->value) where we compare the value to the hash, then when the value matches hash we compare the associated key to the value of a different mapping?  Would be better to just have a Hash->NodeID mapping...
stellar::SCPQuorumSetPtr stellar::TestSCP::getQSet(Hash const& qSetHash) {
/* FROM REFERENCE CODE
        SCPQuorumSetPtr
        TestSCP::getQSet(Hash const& qSetHash)
        {
            // We support exactly 1 qset in this model.
            if (qSetHash != gNetwork->mQSetHash)
            {
                using namespace stellar;
                setLocalLogPrefix();
                CLOG_ERROR(SCP, "can't find qset hash {}", hexAbbrev(qSetHash));
                throw std::runtime_error("can't find qset hash");
            }
            return gNetwork->mQSet;
        }
*/
}

// misc helper
unsigned toIvyValT(stellar::Value const& val) {
    return (unsigned)val.at(0);
}

// ugly global network.  Will have to change this in the future.  
// Have this here for now because I'm afraid changing the signature of TestSCP functions (ex: getQSet)
// that need access to gNetwork will make TestSCP incompatible with stellar-core's SCPDriver
unique_ptr<stellar::Network> gNetwork;

// TODO: implement
void stellar::TestSCP::emitEnvelope(SCPEnvelope const& envelope) {}
stellar::Hash stellar::TestSCP::getHashOf(std::vector<xdr::opaque_vec<>> const& vals) const {} // Compiler wouldn't accept without "const" here
stellar::ValueWrapperPtr stellar::TestSCP::combineCandidates(uint64 slotIndex, ValueWrapperPtrSet const& candidates) {}
void stellar::TestSCP::setupTimer(uint64 slotIndex, int timerID, std::chrono::milliseconds timeout, std::function<void()> cb) {}
void stellar::TestSCP::stopTimer(uint64 slotIndex, int timerID) {}



int main() {

    // node-input file to be parsed
    string filename = "node-input.txt";

    // parse input into a tuple
    tuple<
        xdr::xvector<stellar::NodeID>,
        map<stellar::NodeID, stellar::SCPQuorumSetPtr>,
        map<stellar::NodeID, string>> 
    result = parseInput(filename);

    // unwraps result tuple
    xdr::xvector<stellar::NodeID> node_vec = get<0>(result);
    map<stellar::NodeID, stellar::SCPQuorumSetPtr> node_to_quorum = get<1>(result);
    map<stellar::NodeID, string> node_to_name = get<2>(result);

    cout << "Printing nodes via node_to_name mapping\n";
    for(auto it = node_to_name.cbegin(); it != node_to_name.cend(); ++it) {
        cout << it->second << "\n";
    }

    cout << "\nPrinting nodes via node_vec\n";
    for(stellar::NodeID i: node_vec) {
        cout << node_to_name[i] << "\n";
    }

    cout << "\nPrinting quorum slices\n";
    stellar::SCPQuorumSetPtr curr_quorum;
    stellar::NodeID curr_node;
    for(auto it = node_to_quorum.cbegin(); it != node_to_quorum.cend(); ++it) {
        curr_node = it->first;
        curr_quorum = it->second;
        cout << "Quorum slice for node: " << node_to_name[curr_node] << "\n";
        bool has_validators = false; // Track if any validators are printed
        for(stellar::NodeID i: curr_quorum->validators) {
            if (node_to_name.find(i) != node_to_name.end()) { // Check if the node is valid
                cout << "   " << node_to_name[i] << "\n";
                has_validators = true;
            } else {
                cout << RED << "   [ERROR: Unrecognized node in quorum slice]" << RESET << endl;
                // for(stellar::NodeID j: curr_quorum->validators) {
                //     cout << "NODE_TO_NAME   " << node_to_name[j] << "\n";
                // }
                // cout << "\n";

                has_validators = true;
            }
        }
        if (!has_validators) {
            cout << RED << "   [No validators in quorum slice]" << RESET << endl;
        }
    }


    // Setting up stellar logging.  This is needed for stellar objects
    stellar::Logging::init();
    stellar::Logging::setLoggingColor(true);
    stellar::Logging::setLogLevel(stellar::LogLevel::LVL_TRACE, "SCP");

    // init Network
    gNetwork = make_unique<stellar::Network>(&node_vec, &node_to_quorum);

    // Error checking and debugging.  This should be refactored later
    if(gNetwork && !gNetwork->mNodeIDs.empty() && !gNetwork->mQSet.empty()){

        // Previous error testing.  Might be useful for future debugging
        // const stellar::NodeID test_node = gNetwork->mNodeIDs[0];
        // const stellar::SCPQuorumSet test_quorum = *gNetwork->mQSet.at(test_node);//node_to_quorum.at(node_vec[0]);
        // unique_ptr<stellar::TestSCP> TestSCP_node = make_unique<stellar::TestSCP>(test_node, test_quorum);
    }
    else {
        cout << "Invalid gNetwork object\n";
    }

    unique_ptr<stellar::TestSCP> testSCP[node_vec.size()];
    // Initialize testSCP class for each node and qSet
    for (int i = 0; i < node_vec.size(); i++) {
        testSCP[i] = make_unique<stellar::TestSCP>(gNetwork->mNodeIDs[i], *gNetwork->mQSet.at(gNetwork->mNodeIDs[i]));
    }

    // Now start the actual consensus process
    // have to deal with slots and values

    return 0;
}
