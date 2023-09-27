
using namespace std;

stellar::SCPQuorumSetPtr initQSet(xdr::xvector<stellar::NodeID> const& nodes);

bool checkNodeExists(const string& node_name, const map<stellar::NodeID, string>& node_to_name);

tuple<
    xdr::xvector<stellar::NodeID>,
    map<stellar::NodeID, stellar::SCPQuorumSetPtr>,
    map<stellar::NodeID, string>> 
parseInput(string filename);