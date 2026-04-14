#include "pch.h"
#include "CppUnitTest.h"

#include "../DEORS_Shared/Packet.cpp"
#include "../DEORS_Shared/StateMachine.cpp"
#include "../DEORS_Client/ClientHelpers.cpp"
#include "../DEORS_Server/ServerHelpers.cpp"

#include "Packet.h"
#include "StateMachine.h"
#include "CommandType.h"
#include "ServerState.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DEORSTests
{
    TEST_CLASS(PacketTests)
    {
    public:

        TEST_METHOD(Packet_StringPayload_RoundTrip_Works)
        {
            std::string text = "DECLARE_ALERT";
            std::vector<char> payload = Packet::StringToPayload(text);
            std::string result = Packet::PayloadToString(payload);

            Assert::AreEqual(text, result);
        }

        TEST_METHOD(Packet_SerializeDeserialize_PreservesData)
        {
            std::string text = "LOGIN_REQUEST";
            Packet original(
                CommandType::LOGIN_REQUEST,
                42,
                Packet::StringToPayload(text)
            );

            std::vector<char> raw = original.Serialize();

            Packet parsed;
            bool ok = Packet::Deserialize(raw, parsed);

            Assert::IsTrue(ok);
            Assert::AreEqual((int)CommandType::LOGIN_REQUEST, (int)parsed.commandType);
            Assert::AreEqual(42, parsed.sessionId);
            Assert::AreEqual(text, Packet::PayloadToString(parsed.payload));
        }

        TEST_METHOD(Packet_Deserialize_InvalidData_Fails)
        {
            std::vector<char> badData = { 1, 2, 3, 4, 5 };

            Packet parsed;
            bool ok = Packet::Deserialize(badData, parsed);

            Assert::IsFalse(ok);
        }
    };

    TEST_CLASS(StateMachineTests)
    {
    public:

        TEST_METHOD(StateMachine_InitialState_IsNormal)
        {
            StateMachine sm;
            Assert::AreEqual((int)ServerState::NORMAL, (int)sm.GetCurrentState());
        }

        TEST_METHOD(StateMachine_DeclareAlert_FromNormal_Works)
        {
            StateMachine sm;

            bool canExecute = sm.CanExecute(CommandType::DECLARE_ALERT);
            bool applied = sm.Apply(CommandType::DECLARE_ALERT);

            Assert::IsTrue(canExecute);
            Assert::IsTrue(applied);
            Assert::AreEqual((int)ServerState::ALERT_ACTIVE, (int)sm.GetCurrentState());
        }

        TEST_METHOD(StateMachine_Escalate_FromNormal_Fails)
        {
            StateMachine sm;

            bool canExecute = sm.CanExecute(CommandType::ESCALATE_ALERT);
            bool applied = sm.Apply(CommandType::ESCALATE_ALERT);

            Assert::IsFalse(canExecute);
            Assert::IsFalse(applied);
            Assert::AreEqual((int)ServerState::NORMAL, (int)sm.GetCurrentState());
        }

        TEST_METHOD(StateMachine_FullValidTransitionFlow_Works)
        {
            StateMachine sm;

            Assert::IsTrue(sm.Apply(CommandType::DECLARE_ALERT));
            Assert::AreEqual((int)ServerState::ALERT_ACTIVE, (int)sm.GetCurrentState());

            Assert::IsTrue(sm.Apply(CommandType::ESCALATE_ALERT));
            Assert::AreEqual((int)ServerState::ESCALATED, (int)sm.GetCurrentState());

            Assert::IsTrue(sm.Apply(CommandType::RESOLVE_ALERT));
            Assert::AreEqual((int)ServerState::RESOLVED, (int)sm.GetCurrentState());

            Assert::IsTrue(sm.Apply(CommandType::RESET_SYSTEM));
            Assert::AreEqual((int)ServerState::NORMAL, (int)sm.GetCurrentState());
        }



        TEST_METHOD(StateMachine_GetStateAsString_Initial_IsNormal)
        {
            StateMachine sm;
            Assert::IsTrue(sm.GetStateAsString() == "NORMAL");
        }

        TEST_METHOD(StateMachine_GetStateAsString_AfterDeclare_IsAlertActive)
        {
            StateMachine sm;
            sm.Apply(CommandType::DECLARE_ALERT);

            Assert::IsTrue(sm.GetStateAsString() == "ALERT_ACTIVE");
        }

        TEST_METHOD(StateMachine_GetStateAsString_AfterEscalate_IsEscalated)
        {
            StateMachine sm;
            sm.Apply(CommandType::DECLARE_ALERT);
            sm.Apply(CommandType::ESCALATE_ALERT);

            Assert::IsTrue(sm.GetStateAsString() == "ESCALATED");
        }

        TEST_METHOD(StateMachine_GetStateAsString_AfterResolve_IsResolved)
        {
            StateMachine sm;
            sm.Apply(CommandType::DECLARE_ALERT);
            sm.Apply(CommandType::ESCALATE_ALERT);
            sm.Apply(CommandType::RESOLVE_ALERT);

            Assert::IsTrue(sm.GetStateAsString() == "RESOLVED");
        }


        TEST_METHOD(Packet_PayloadSize_Zero_WhenEmpty)
        {
            Packet pkt(
                CommandType::LOGIN_REQUEST,
                1,
                std::vector<char>()
            );

            Assert::AreEqual(0, (int)pkt.payloadSize);
        }

        TEST_METHOD(Packet_StringToPayload_SizeMatches)
        {
            std::string text = "ESCALATE_ALERT";
            std::vector<char> payload = Packet::StringToPayload(text);

            Assert::AreEqual((int)text.size(), (int)payload.size());
        }

        TEST_METHOD(Packet_PayloadToString_Empty_ReturnsEmpty)
        {
            std::vector<char> payload;
            std::string result = Packet::PayloadToString(payload);

            Assert::IsTrue(result == "");
        }

        TEST_METHOD(Packet_SessionId_Preserved_AfterDeserialize)
        {
            Packet original(
                CommandType::DECLARE_ALERT,
                999,
                Packet::StringToPayload("test")
            );

            std::vector<char> raw = original.Serialize();

            Packet parsed;
            bool ok = Packet::Deserialize(raw, parsed);

            Assert::IsTrue(ok);
            Assert::AreEqual(999, parsed.sessionId);
        }



        TEST_METHOD(StateMachine_CanExecute_DeclareAlert_FromNormal)
        {
            StateMachine sm;
            Assert::IsTrue(sm.CanExecute(CommandType::DECLARE_ALERT));
        }

        TEST_METHOD(StateMachine_CanExecute_Escalate_FromNormal_False)
        {
            StateMachine sm;
            Assert::IsFalse(sm.CanExecute(CommandType::ESCALATE_ALERT));
        }

        TEST_METHOD(StateMachine_CanExecute_Resolve_FromEscalated)
        {
            StateMachine sm;
            sm.Apply(CommandType::DECLARE_ALERT);
            sm.Apply(CommandType::ESCALATE_ALERT);

            Assert::IsTrue(sm.CanExecute(CommandType::RESOLVE_ALERT));
        }

        TEST_METHOD(StateMachine_Reset_FromResolved_Works)
        {
            StateMachine sm;
            sm.Apply(CommandType::DECLARE_ALERT);
            sm.Apply(CommandType::ESCALATE_ALERT);
            sm.Apply(CommandType::RESOLVE_ALERT);

            bool result = sm.Apply(CommandType::RESET_SYSTEM);

            Assert::IsTrue(result);
            Assert::IsTrue(sm.GetStateAsString() == "NORMAL");
        }

        TEST_METHOD(ClientHelper_ValidMenuOption_0_IsTrue)
        {
            Assert::IsTrue(IsValidMenuOptionHelper(0));
        }

        TEST_METHOD(ClientHelper_ValidMenuOption_6_IsTrue)
        {
            Assert::IsTrue(IsValidMenuOptionHelper(6));
        }

        TEST_METHOD(ClientHelper_InvalidMenuOption_Negative_IsFalse)
        {
            Assert::IsFalse(IsValidMenuOptionHelper(-1));
        }

        TEST_METHOD(ClientHelper_InvalidMenuOption_TooHigh_IsFalse)
        {
            Assert::IsFalse(IsValidMenuOptionHelper(7));
        }

        TEST_METHOD(ClientHelper_Choice2_MapsToDeclareAlert)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(2) == "DECLARE_ALERT");
        }

        TEST_METHOD(ClientHelper_Choice3_MapsToEscalateAlert)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(3) == "ESCALATE_ALERT");
        }

        TEST_METHOD(ClientHelper_Choice6_MapsToRequestSituationReport)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(6) == "REQUEST_SITUATION_REPORT");
        }

        TEST_METHOD(ClientHelper_InvalidChoice_ReturnsEmptyString)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(99) == "");
        }

        TEST_METHOD(ServerHelper_ValidCredential_ReturnsTrue)
        {
            Assert::IsTrue(IsValidCredentialHelper("aya:1234"));
        }

        TEST_METHOD(ServerHelper_InvalidCredential_ReturnsFalse)
        {
            Assert::IsFalse(IsValidCredentialHelper("aya:wrong"));
        }

        TEST_METHOD(ServerHelper_ParseDeclareAlert_Works)
        {
            Assert::AreEqual((int)CommandType::DECLARE_ALERT, (int)ParseCommandFromTextHelper("DECLARE_ALERT"));
        }

        TEST_METHOD(ServerHelper_ParseRequestSituationReport_Works)
        {
            Assert::AreEqual((int)CommandType::REQUEST_SITUATION_REPORT, (int)ParseCommandFromTextHelper("REQUEST_SITUATION_REPORT"));
        }

        TEST_METHOD(ServerHelper_ParseUnknownCommand_ReturnsErrorResponse)
        {
            Assert::AreEqual((int)CommandType::ERROR_RESPONSE, (int)ParseCommandFromTextHelper("BAD_COMMAND"));
        }


        TEST_METHOD(ClientHelper_Choice1_ReturnsEmpty)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(1) == "");
        }

        TEST_METHOD(ClientHelper_Choice0_ReturnsEmpty)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(0) == "");
        }

        TEST_METHOD(ClientHelper_LargeChoice_ReturnsEmpty)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(999) == "");
        }



        TEST_METHOD(ServerHelper_EmptyCredential_Fails)
        {
            Assert::IsFalse(IsValidCredentialHelper(""));
        }

        TEST_METHOD(ServerHelper_NoColonCredential_Fails)
        {
            Assert::IsFalse(IsValidCredentialHelper("aya1234"));
        }

        TEST_METHOD(ServerHelper_OnlyColonCredential_Fails)
        {
            Assert::IsFalse(IsValidCredentialHelper(":"));
        }


        TEST_METHOD(ServerHelper_LowercaseCommand_ReturnsError)
        {
            Assert::AreEqual((int)CommandType::ERROR_RESPONSE,
                (int)ParseCommandFromTextHelper("declare_alert"));
        }

        TEST_METHOD(ServerHelper_EmptyCommand_ReturnsError)
        {
            Assert::AreEqual((int)CommandType::ERROR_RESPONSE,
                (int)ParseCommandFromTextHelper(""));
        }


        TEST_METHOD(StateMachine_InvalidTransition_EscalateFromNormal_Fails)
        {
            StateMachine sm;
            Assert::IsFalse(sm.CanExecute(CommandType::ESCALATE_ALERT));
        }



        TEST_METHOD(ClientHelper_Choice4_MapsToResolveAlert)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(4) == "RESOLVE_ALERT");
        }

        TEST_METHOD(ClientHelper_Choice5_MapsToResetSystem)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(5) == "RESET_SYSTEM");
        }

        TEST_METHOD(ClientHelper_NegativeChoice_ReturnsEmpty)
        {
            Assert::IsTrue(GetCommandTextFromChoiceHelper(-5) == "");
        }

        TEST_METHOD(ServerHelper_ParseEscalateAlert_Works)
        {
            Assert::AreEqual((int)CommandType::ESCALATE_ALERT,
                (int)ParseCommandFromTextHelper("ESCALATE_ALERT"));
        }

        TEST_METHOD(ServerHelper_ParseResolveAlert_Works)
        {
            Assert::AreEqual((int)CommandType::RESOLVE_ALERT,
                (int)ParseCommandFromTextHelper("RESOLVE_ALERT"));
        }

        TEST_METHOD(ServerHelper_ParseResetSystem_Works)
        {
            Assert::AreEqual((int)CommandType::RESET_SYSTEM,
                (int)ParseCommandFromTextHelper("RESET_SYSTEM"));
        }

        TEST_METHOD(ServerHelper_Credential_CaseSensitive_Fails)
        {
            Assert::IsFalse(IsValidCredentialHelper("Aya:1234"));
        }

        TEST_METHOD(ServerHelper_Credential_WrongUsername_Fails)
        {
            Assert::IsFalse(IsValidCredentialHelper("lanna:1234"));
        }

        TEST_METHOD(ServerHelper_Credential_Whitespace_Fails)
        {
            Assert::IsFalse(IsValidCredentialHelper(" aya:1234 "));
        }

        TEST_METHOD(Packet_StringToPayload_SpacesPreserved)
        {
            std::string text = "DECLARE ALERT NOW";
            std::vector<char> payload = Packet::StringToPayload(text);
            std::string result = Packet::PayloadToString(payload);

            Assert::IsTrue(result == text);
        }

        TEST_METHOD(Packet_StringToPayload_SpecialCharactersPreserved)
        {
            std::string text = "aya:1234!@#";
            std::vector<char> payload = Packet::StringToPayload(text);
            std::string result = Packet::PayloadToString(payload);

            Assert::IsTrue(result == text);
        }

        TEST_METHOD(Packet_PayloadSize_MatchesVectorSize_NonEmpty)
        {
            std::vector<char> payload = { 'X', 'Y', 'Z' };
            Packet pkt(
                CommandType::STATE_UPDATE,
                5,
                payload
            );

            Assert::AreEqual(3, (int)pkt.payloadSize);
        }

        TEST_METHOD(Packet_SerializeDeserialize_DeclareAlertCommand_Preserved)
        {
            Packet original(
                CommandType::DECLARE_ALERT,
                11,
                Packet::StringToPayload("DECLARE_ALERT")
            );

            std::vector<char> raw = original.Serialize();

            Packet parsed;
            bool ok = Packet::Deserialize(raw, parsed);

            Assert::IsTrue(ok);
            Assert::AreEqual((int)CommandType::DECLARE_ALERT, (int)parsed.commandType);
        }

        TEST_METHOD(Packet_SerializeDeserialize_RequestReportCommand_Preserved)
        {
            Packet original(
                CommandType::REQUEST_SITUATION_REPORT,
                12,
                Packet::StringToPayload("REQUEST_SITUATION_REPORT")
            );

            std::vector<char> raw = original.Serialize();

            Packet parsed;
            bool ok = Packet::Deserialize(raw, parsed);

            Assert::IsTrue(ok);
            Assert::AreEqual((int)CommandType::REQUEST_SITUATION_REPORT, (int)parsed.commandType);
        }

        TEST_METHOD(Packet_SerializeDeserialize_ErrorResponseCommand_Preserved)
        {
            Packet original(
                CommandType::ERROR_RESPONSE,
                13,
                Packet::StringToPayload("ERROR")
            );

            std::vector<char> raw = original.Serialize();

            Packet parsed;
            bool ok = Packet::Deserialize(raw, parsed);

            Assert::IsTrue(ok);
            Assert::AreEqual((int)CommandType::ERROR_RESPONSE, (int)parsed.commandType);
        }

        TEST_METHOD(Packet_Deserialize_TruncatedPacket_Fails)
        {
            Packet original(
                CommandType::LOGIN_REQUEST,
                1,
                Packet::StringToPayload("aya:1234")
            );

            std::vector<char> raw = original.Serialize();
            raw.pop_back();

            Packet parsed;
            bool ok = Packet::Deserialize(raw, parsed);

            Assert::IsFalse(ok);
        }

        TEST_METHOD(StateMachine_CanExecute_Reset_FromResolved_True)
        {
            StateMachine sm;
            sm.Apply(CommandType::DECLARE_ALERT);
            sm.Apply(CommandType::ESCALATE_ALERT);
            sm.Apply(CommandType::RESOLVE_ALERT);

            Assert::IsTrue(sm.CanExecute(CommandType::RESET_SYSTEM));
        }

        TEST_METHOD(StateMachine_DeclareAfterReset_Works)
        {
            StateMachine sm;
            sm.Apply(CommandType::DECLARE_ALERT);
            sm.Apply(CommandType::ESCALATE_ALERT);
            sm.Apply(CommandType::RESOLVE_ALERT);
            sm.Apply(CommandType::RESET_SYSTEM);

            bool result = sm.Apply(CommandType::DECLARE_ALERT);

            Assert::IsTrue(result);
            Assert::IsTrue(sm.GetStateAsString() == "ALERT_ACTIVE");
        }

        TEST_METHOD(StateMachine_CanExecute_Declare_FromAlertActive_False)
        {
            StateMachine sm;
            sm.Apply(CommandType::DECLARE_ALERT);

            Assert::IsFalse(sm.CanExecute(CommandType::DECLARE_ALERT));
        }

    };
}