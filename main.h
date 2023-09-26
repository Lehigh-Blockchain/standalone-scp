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

namespace stellar {

    // There will be 1 instance of this Network object representing the network
    // as a whole. We have to initialize this all in one (global) place so we can
    // build a single QSet that contains all the NodeIDs. It's constant once
    // initialized.
    struct Network {
        // The network has a fixed set of nodes.
        const xdr::xvector<NodeID> mNodeIDs;

        /*
        The reference code handles only one quorum set for the whole network like so:
        const SCPQuorumSetPtr mQSet;

        but for our code we want to be able to handle multiple quorum sets, so we will have
        a mapping of nodeIDs to quorum sets for that nodeID
        */
        std::map<NodeID, SCPQuorumSetPtr> mQSet;
        
        // reference-code handles only a single qSet, but since we want to be able to handle a dynamic amount of
        // qSets, we have to handle multiple Hashes
        // TODO: should this be a mapping SCPQuorumSetPtr->Hash or NodeID->Hash? 
        // Would have to change computeQSetHash function and anywhere that uses mQSetHash
        const std::map<SCPQuorumSetPtr, Hash> mQSetHash;

        // Envelopes are broadcast into a globally visible history which
        // individual nodes advance their consumption of.
        std::vector<SCPEnvelopeWrapperPtr> mBroadcastEnvelopes;

        Network(const xdr::xvector<NodeID> *node_vec, const std::map<NodeID, SCPQuorumSetPtr> *node_quorum_map);
    };

    class TestSCP : public SCPDriver {
        SCP mSCP;

        // pure virtual functions that have to have an implementation
        void signEnvelope(SCPEnvelope& envelope) override;
        SCPQuorumSetPtr getQSet(Hash const& qSetHash);
        void emitEnvelope(SCPEnvelope const& envelope) override;
        Hash getHashOf(std::vector<xdr::opaque_vec<>> const& vals) const override;
        ValueWrapperPtr combineCandidates(uint64 slotIndex, ValueWrapperPtrSet const& candidates) override;
        void setupTimer(uint64 slotIndex, int timerID, std::chrono::milliseconds timeout, std::function<void()> cb) override;
        void stopTimer(uint64 slotIndex, int timerID) override;

        public:
        TestSCP(NodeID const& nodeID, SCPQuorumSet const& qSetLocal);
        SCP &getSCP() { return mSCP; }

        // Miscellaneous helper functions used in the implementation, that happen to need
        // to be members of TestSCP in order to access (possibly private) state in SCP.
        void setLocalLogPrefix();

    };
    
    // Each node will have an instance of this class which contains the SCP state machine
    // and also is an SCPDriver which mediates communication between Ivy and that SCP.
    //
    // NB: this class has to be called stellar__COLON____COLON__TestSCP exactly, to match a friend
    // declaration in SCP that allows us to access private members of SCP.

    /*
    class TestSCP : public SCPDriver {

        SCP mSCP;

        // TODO: implement these functions
        void signEnvelope(SCPEnvelope& envelope) override;
        
        SCPQuorumSetPtr getQSet(Hash const& qSetHash) override;
        void emitEnvelope(SCPEnvelope const& envelope) override;
        Hash getHashOf(std::vector<xdr::opaque_vec<>> const& vals) const override;
        ValueWrapperPtr combineCandidates(uint64 slotIndex, ValueWrapperPtrSet const& candidates) override;
        void setupTimer(uint64 slotIndex, int timerID, std::chrono::milliseconds timeout, std::function<void()> cb) override;
        ValidationLevel validateValue(uint64 slotIndex, Value const& value, bool nomination) override;

        // Hooks that are fired when various SCP state-transitions occur. These are routed
        // into the callbacks that follow.
        void valueExternalized(uint64 slotIndex, Value const& value) override;
        void nominatingValue(uint64 slotIndex, Value const& value) override;
        void startedBallotProtocol(uint64 slotIndex, SCPBallot const& ballot) override;
        void acceptedBallotPrepared(uint64 slotIndex, SCPBallot const& ballot) override;
        void confirmedBallotPrepared(uint64 slotIndex, SCPBallot const& ballot) override;
        void acceptedCommit(uint64 slotIndex, SCPBallot const& ballot) override;

        // Thunks that can be set for callbacks from the SCP driver to Ivy.
        
        // std::unique_ptr<thunk__node__value_externalized> mValueExternalizedCb;
        // std::unique_ptr<thunk__node__nominating_value> mNominatingValueCb;
        // std::unique_ptr<thunk__node__started_ballot_protocol> mStartedBallotProtocolCb;
        // std::unique_ptr<thunk__node__accepted_ballot_prepared> mAcceptedBallotPreparedCb;
        // std::unique_ptr<thunk__node__confirmed_ballot_prepared> mConfirmedBallotPreparedCb;
        // std::unique_ptr<thunk__node__accepted_commit> mAcceptedCommitCb;
        // std::unique_ptr<thunk__node__emit_envelope> mEmitEnvelopeCb;
        // std::unique_ptr<thunk__node__emit_prepare> mEmitPrepareCb;
        // std::unique_ptr<thunk__node__emit_confirm> mEmitConfirmCb;
        // std::unique_ptr<thunk__node__emit_externalize> mEmitExternalizeCb;
        // std::unique_ptr<thunk__node__emit_logical_vote_nominate> mEmitLogicalVoteNominateCb;
        // std::unique_ptr<thunk__node__emit_logical_accept_nominate> mEmitLogicalAcceptNominateCb;
        
        public:

        // TestSCP(NodeID const& nodeID, SCPQuorumSet const& qSetLocal,
        // std::unique_ptr<thunk__node__value_externalized>,
        // std::unique_ptr<thunk__node__nominating_value>,
        // std::unique_ptr<thunk__node__started_ballot_protocol>,
        // std::unique_ptr<thunk__node__accepted_ballot_prepared>,
        // std::unique_ptr<thunk__node__confirmed_ballot_prepared>,
        // std::unique_ptr<thunk__node__accepted_commit>,
        // std::unique_ptr<thunk__node__emit_envelope>,
        // std::unique_ptr<thunk__node__emit_prepare>,
        // std::unique_ptr<thunk__node__emit_confirm>,
        // std::unique_ptr<thunk__node__emit_externalize>,
        // std::unique_ptr<thunk__node__emit_logical_vote_nominate>,
        // std::unique_ptr<thunk__node__emit_logical_accept_nominate>
        
        );

        SCP &getSCP() { return mSCP; }

        // Miscellaneous helper functions used in the implementation, that happen to need
        // to be members of TestSCP in order to access (possibly private) state in SCP.
        void setLocalLogPrefix();
        bool isLeader();
        unsigned getSCPStatementCount();
        unsigned getNomRound();
        template <typename T>
        T getSCPBallotState(std::function<T(std::string const&)> mapping) {
            setLocalLogPrefix();
            return mapping(mSCP.getSlot(0, true)->getBallotProtocol().getJsonInfo()["phase"].asString());
        }
        
    };
    */
}

//the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

// https://stackoverflow.com/questions/9158150/colored-output-in-c
