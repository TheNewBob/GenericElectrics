#include "stdafx.h"
#include "CppUnitTest.h"
#include "stdincludes.h"
#include "TestUtils.h"
#include "PowerTypes.h"
#include "PowerChild.h"
#include "PowerParent.h"
#include "PowerConsumer.h"
#include "PowerBus.h"
#include "PowerSource.h"
#include "PowerSourceChargable.h"
#include "PowerConverter.h"
#include "PowerCircuit_Base.h"
#include "PowerCircuit.h"
#include "PowerCircuitManager.h"
//#include "Calc.h"
#include <time.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace IMS2Unit
{
	TEST_CLASS(PowerCircuitTest)
	{
	public:


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_ForbiddenConnectionsTest)
			TEST_DESCRIPTION(L"Tests if forbidden connections can't be formed.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_ForbiddenConnectionsTest)
		{
			Logger::WriteMessage(L"\n\nTest: ForbiddenConnectionsTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();
			PowerConsumer *consumerlowvoltage = new PowerConsumer(5, 25, 100, 0);
			PowerConsumer *consumerhighvoltage = new PowerConsumer(27, 80, 100, 0);
			PowerBus *bus = new PowerBus(26, 1000, manager, 0);
			PowerSource *sourcelowvoltage = new PowerSource(5, 25, 200, 1, 0);
			PowerSource *sourcehighvoltage = new PowerSource(5, 25, 200, 1, 0);

			Logger::WriteMessage(L"Testing connection of invalid types\n");
			Assert::IsFalse(consumerlowvoltage->CanConnectToParent(sourcelowvoltage), L"consumer must NOT be able to connect to source!");
			Assert::IsFalse(sourcelowvoltage->CanConnectToChild(consumerlowvoltage), L"source must NOT be able to connect to consumer!");

			Logger::WriteMessage(L"Testing connection consumer - bus with incompatible voltages\n");
			Assert::IsFalse(consumerlowvoltage->CanConnectToParent(bus), L"consumer with lower voltage range must NOT able to connect to bus outside voltage range!");
			Assert::IsFalse(consumerhighvoltage->CanConnectToParent(bus), L"consumer with high voltage range must NOT be able to connect to bus outside voltage range!");
			Assert::IsFalse(bus->CanConnectToChild(consumerlowvoltage), L"bus above voltage range must NOT able to connect to consumer!");
			Assert::IsFalse(bus->CanConnectToChild(consumerhighvoltage), L"bus below voltage range must NOT able to connect to consumer!");

			Logger::WriteMessage(L"Testing connection source - bus with incompatible voltages\n");
			Assert::IsFalse(sourcelowvoltage->CanConnectToChild(bus), L"source with lower voltage range must NOT able to connect to bus outside voltage range!");
			Assert::IsFalse(sourcehighvoltage->CanConnectToChild(bus), L"source with high voltage range must NOT be able to connect to bus outside voltage range!");
			Assert::IsFalse(bus->CanConnectToParent(sourcelowvoltage), L"bus above voltage range must NOT able to connect to source!");
			Assert::IsFalse(bus->CanConnectToParent(sourcehighvoltage), L"bus below voltage range must NOT able to connect to source!");

			Logger::WriteMessage(L"Testing bus connections with self\n");
			Assert::IsFalse(bus->CanConnectToChild(bus), L"bus must NOT be able to be its own parent!");
			Assert::IsFalse(bus->CanConnectToParent(bus), L"bus must NOT be able to be its own child!");

			Logger::WriteMessage(L"Testing circular bus connections\n");
			PowerBus *bus2 = new PowerBus(26, 1000, manager, 0);
			PowerBus *bus3 = new PowerBus(26, 1000, manager, 0);
			bus2->ConnectChildToParent(bus);
			bus3->ConnectChildToParent(bus2);
			Assert::IsFalse(bus3->CanConnectToChild(bus), L"Circular bus connection must NOT be possible!");
			Assert::IsFalse(bus->CanConnectToChild(bus3), L"Circular bus connection must NOT be possible");
			Assert::IsFalse(bus3->CanConnectToParent(bus), L"Circular bus connection must NOT be possible!");
			Assert::IsFalse(bus->CanConnectToParent(bus3), L"Circular bus connection must NOT be possible");

			Logger::WriteMessage(L"Testing invalid multi-connections\n");
			PowerBus *bus4 = new PowerBus(26, 100, manager, 0);
			PowerSource *source = new PowerSource(20, 50, 200, 1, 0);
			bus4->ConnectChildToParent(source);
			Assert::IsFalse(bus->CanConnectToParent(source), L"Powersource must NOT allow connection to more than 1 bus!");
			Assert::IsFalse(source->CanConnectToChild(bus), L"Powersource must NOT allow connection to more than 1 bus!");

			PowerConsumer *consumer = new PowerConsumer(20, 30, 100, 0);
			consumer->ConnectChildToParent(bus4);
			Assert::IsFalse(consumer->CanConnectToParent(bus3), L"Consumer must NOT allow connection to more than 1 bus!");

			Logger::WriteMessage(L"cleaning up test assets\n");
			delete bus4;
			delete bus3;
			delete bus2;
			delete bus;
			delete source;
			delete consumerhighvoltage;
			delete consumerlowvoltage;
			delete sourcehighvoltage;
			delete sourcelowvoltage;
			delete consumer;
			delete manager;
		}


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_SimpleCircuitConnectionTest)
			TEST_DESCRIPTION(L"Tests connecting a bus, a consumer and a source.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_SimpleCircuitConnectionTest)
		{
			Logger::WriteMessage(L"\n\nTest: SimpleCircuitConnectionTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();
			PowerConsumer *consumer = new PowerConsumer(15, 30, 100, 0);
			PowerBus *bus = new PowerBus(26, 1000, manager, 0);
			PowerSource *source = new PowerSource(15, 30, 200, 1, 0);

			Logger::WriteMessage(L"Testing connection checks\n");
			Assert::IsTrue(bus->CanConnectToChild(consumer), TestUtils::Msg("Bus must be able to connect to consumer!"));
			Assert::IsTrue(bus->CanConnectToParent(source), TestUtils::Msg("Bus must be able to connect to source!"));
			Assert::IsTrue(source->CanConnectToChild(bus), TestUtils::Msg("source must be able to connect to bus!"));
			Assert::IsTrue(consumer->CanConnectToParent(bus), TestUtils::Msg("consumer must be able to connect to bus!"));

			source->ConnectParentToChild(bus);
			consumer->ConnectChildToParent(bus);

			Logger::WriteMessage(L"Testing circuit creation\n");
			Assert::IsTrue(manager->GetSize() == 1);

			Logger::WriteMessage(L"Testing circuit assignement\n");
			Assert::IsNotNull(source->GetCircuit(), TestUtils::Msg("source circuit is NULL!"));
			Assert::IsNotNull(bus->GetCircuit(), TestUtils::Msg("bus circuit is NULL!"));
			Assert::IsTrue(source->GetCircuit() == bus->GetCircuit(), TestUtils::Msg("bus and source are not in same circuit!"));

			vector<PowerChild*> checkchildren;
			source->GetChildren(checkchildren);
			Logger::WriteMessage(TestUtils::Msg("Number of children of source: " + to_string(checkchildren.size()) + "\n"));
			Assert::IsTrue(checkchildren.size() == 1, TestUtils::Msg("source has wrong number of children!"));

			bus->GetChildren(checkchildren);
			Logger::WriteMessage(TestUtils::Msg("Number of children of bus: " + to_string(checkchildren.size()) + "\n"));
			Assert::IsTrue(checkchildren.size() == 1, TestUtils::Msg("bus has wrong number of children!"));

			vector<PowerParent*> checkparents;
			consumer->GetParents(checkparents);
			Logger::WriteMessage(TestUtils::Msg("Number of parents of consumer: " + to_string(checkparents.size()) + "\n"));
			Assert::IsTrue(checkparents.size() == 1, TestUtils::Msg("consumer has wrong number of parents!"));
			
			bus->GetParents(checkparents);
			Logger::WriteMessage(TestUtils::Msg("Number of parents of bus: " + to_string(checkchildren.size()) + "\n"));
			Assert::IsTrue(checkchildren.size() == 1, TestUtils::Msg("bus has wrong number of children!"));
		
			Logger::WriteMessage(L"cleaning up test assets\n");
			delete consumer;
			delete source;
			delete bus;
			delete manager;
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(Power_SimpleCircuitEvaluationTest)
			TEST_DESCRIPTION(L"Tests evaluation of a simple circuit.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_SimpleCircuitEvaluationTest)
		{
			Logger::WriteMessage(L"\n\nTest: SimpleCircuitConnectionTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();
			PowerConsumer *consumer = new PowerConsumer(15, 30, 100, 0);
			PowerBus *bus = new PowerBus(26, 1000, manager, 0);
			PowerSource *source = new PowerSource(15, 30, 200, 1, 0);

			source->ConnectParentToChild(bus);
			consumer->ConnectChildToParent(bus);

			Logger::WriteMessage(L"Testing circuit evaluation at full load\n");
			consumer->SetConsumerLoad(1);
			manager->Evaluate(1);

			vector<PowerCircuit*> circuits;
			manager->GetPowerCircuits(circuits);
			PowerCircuit *circuit = circuits[0];

			Logger::WriteMessage(TestUtils::Msg("current input of consumer at current voltage: " + to_string(consumer->GetInputCurrent()) + "\n"));
			Assert::IsTrue(consumer->GetInputCurrent() == 3.8461538461538461538461538461538, L"Consumer does not have correct input current!");

			Logger::WriteMessage(TestUtils::Msg("equivalent resistance of circuit: " + to_string(circuit->GetEquivalentResistance()) + "\n"));
			Assert::IsTrue(circuit->GetEquivalentResistance() == 6.76, TestUtils::Msg("circuit has wrong equivalent resistance!"));
			Logger::WriteMessage(TestUtils::Msg("Total current in circuit: " + to_string(circuit->GetCircuitCurrent()) + "\n"));
			Assert::IsTrue(circuit->GetCircuitCurrent() == 3.8461538461538461538461538461538, TestUtils::Msg("circuit has wrong current flow!"));

			Logger::WriteMessage(L"Testing powersource evaluation at full load\n");
			Logger::WriteMessage(TestUtils::Msg("Current voltage of powersource: " + to_string(source->GetCurrentOutputVoltage()) + "\n"));
			Assert::IsTrue(source->GetCurrentOutputVoltage() == 26, L"Powersource does not have same voltage as bus!");
			Logger::WriteMessage(TestUtils::Msg("Maximum current output of powersource at current voltage: " + to_string(source->GetMaxOutputCurrent()) + "\n"));
			Assert::IsTrue(source->GetMaxOutputCurrent() == 7.6923076923076923076923076923077, L"Powersource does not have correct maximum output current!");
			Logger::WriteMessage(TestUtils::Msg("Current current output of powersource: " + to_string(source->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(source->GetOutputCurrent() == circuit->GetCircuitCurrent(), L"Powersource does not have correct output current!");

			Logger::WriteMessage(TestUtils::Msg("Current current flow through bus: " + to_string(bus->GetCurrent()) + "\n"));
			Assert::IsTrue(source->GetOutputCurrent() == bus->GetCurrent(), L"Incorrect current flowing through bus");


			Logger::WriteMessage(L"Testing circuit evaluation at half load\n");
			consumer->SetConsumerLoad(0.5);
			manager->Evaluate(1);

			Logger::WriteMessage(TestUtils::Msg("current input of consumer at current voltage: " + to_string(consumer->GetInputCurrent()) + "\n"));
			Assert::IsTrue(consumer->GetInputCurrent() == 1.9230769230769230769230769230769, L"Consumer does not have correct input current!");

			Logger::WriteMessage(TestUtils::Msg("equivalent resistance of circuit: " + to_string(circuit->GetEquivalentResistance()) + "\n"));
			Assert::IsTrue(circuit->GetEquivalentResistance() == 13.52, TestUtils::Msg("circuit has wrong equivalent resistance!"));
			Logger::WriteMessage(TestUtils::Msg("Total current in circuit: " + to_string(circuit->GetCircuitCurrent()) + "\n"));
			Assert::IsTrue(circuit->GetCircuitCurrent() == 1.9230769230769230769230769230769, TestUtils::Msg("circuit has wrong current flow!"));

			Logger::WriteMessage(TestUtils::Msg("Current current output of powersource: " + to_string(source->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(source->GetOutputCurrent() == circuit->GetCircuitCurrent(), L"Powersource does not have correct maximum output current!");

			Logger::WriteMessage(TestUtils::Msg("Current current flow through bus: " + to_string(bus->GetCurrent()) + "\n"));
			Assert::IsTrue(source->GetOutputCurrent() == bus->GetCurrent(), L"Incorrect current flowing through bus");


			Logger::WriteMessage(L"cleaning up test assets\n");
			delete consumer;
			delete source;
			delete bus;
			delete circuit;
			delete manager;
		}


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_SimpleCircuitDisconnectionTest)
			TEST_DESCRIPTION(L"Tests removing a bus from a circuit.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_SimpleCircuitDisconnectionTest)
		{
			Logger::WriteMessage(L"\n\nTest: SimpleCircuitConnectionTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();
			PowerConsumer *consumer1 = new PowerConsumer(15, 30, 100, 0);
			PowerConsumer *consumer2 = new PowerConsumer(15, 30, 100, 0);
			PowerBus *bus1 = new PowerBus(26, 1000, manager, 0);
			PowerBus *bus2 = new PowerBus(26, 1000, manager, 0);
			PowerSource *source = new PowerSource(15, 30, 200, 1, 0);

			source->ConnectParentToChild(bus1);
			bus1->ConnectParentToChild(bus2);
			consumer1->ConnectChildToParent(bus1);
			consumer2->ConnectChildToParent(bus2);
			consumer1->SetConsumerLoad(1);
			consumer2->SetConsumerLoad(1);

			manager->Evaluate(1);

			//disconnecting bus2
			bus2->DisconnectChildFromParent(bus1);
			manager->Evaluate(1);
			
			vector<PowerCircuit*> circuits;
			manager->GetPowerCircuits(circuits);

			Logger::WriteMessage(L"Testing circuits after disconnecting bus2\n");

			Assert::IsTrue(circuits.size() == 2, L"Should have two circuits after disconnect!");
			Assert::IsTrue(circuits[0]->GetCircuitCurrent() == 0, L"Incorrect current flowing through circuit 0 after disconnect (should be 0!)");
			Assert::IsTrue(TestUtils::IsEqual(circuits[1]->GetCircuitCurrent(), 100 / 26.0), L"Incorrect current in circuit 1 after disconnect!");
			Assert::IsTrue(source->GetCurrentPowerOutput() == 100, L"Incorrect power output of source (should be 100!)");

			//reconnect the parts
			bus1->ConnectParentToChild(bus2);
			consumer2->SetConsumerLoad(1);
			manager->Evaluate(1);

			//disconnecting at bus 1
			bus1->DisconnectChildFromParent(source);
			manager->Evaluate(1);
			manager->GetPowerCircuits(circuits);

			Logger::WriteMessage(L"Testing circuits after disconnecting source from otherwise intact circuit.\n");

			Assert::IsTrue(circuits.size() == 1, L"Should have one circuit after disconnecting source!");
			Assert::IsTrue(circuits[0]->GetCircuitCurrent() == 0, L"Incorrect current flowing through circuit 0 after disconnecting source (should be 0!)");
			Assert::IsTrue(source->GetCurrentPowerOutput() == 0, L"Incorrect power output of source (should be 0!)");


		}


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_RechargableSourceTest)
			TEST_DESCRIPTION(L"Tests if a chargable powersource behaves as expected.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_RechargableSourceTest)
		{
			Logger::WriteMessage(L"\n\nTest: Power_RechargableSourceTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();
			PowerConsumer *consumer1 = new PowerConsumer(15, 30, 60, 0);
			PowerBus *bus = new PowerBus(26, 1000, manager, 0);
			PowerSourceChargable *chargablesource = new PowerSourceChargable(15, 30, 100, 200, 1000, 0.9, 1, 0, 0.2);
			PowerSource *source = new PowerSource(15, 30, 300, 1, 0);

			source->ConnectParentToChild(bus);
			chargablesource->ConnectParentToChild(bus);
			consumer1->ConnectChildToParent(bus);
			source->SetParentSwitchedIn(false);

			Logger::WriteMessage(L"Testing at full load\n");
			consumer1->SetConsumerLoad(1);
			manager->Evaluate(1);

			Logger::WriteMessage(TestUtils::Msg("current output of chargable source at current voltage: " + to_string(chargablesource->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(chargablesource->GetOutputCurrent(), 60 / 26.0), L"chargable source does not provide expected current!");
			Assert::IsTrue(TestUtils::IsEqual(chargablesource->GetCharge(), 1000 - (60.0 / 3600000)), L"chargable source did not lose the expected amount of charge!");

			Logger::WriteMessage(L"Testing running the source dry\n");
			int seconds = 0; //one second already passed for the last evaluation
			while (chargablesource->IsParentSwitchedIn() && seconds <= 60000)
			{
				seconds++;
				manager->Evaluate(1000);
			}

			Assert::IsTrue(seconds == 60000, 
						   TestUtils::Msg("chargable source should have been able to provide power for exactly 60000 miliseconds, instead provided for " + to_string(seconds) + " miliseconds\n"));
			Assert::IsFalse(chargablesource->IsParentSwitchedIn(), L"Chargable source did not stop providing power as expected!");
			Assert::IsTrue(chargablesource->IsChildSwitchedIn(), L"Chargable source did not switch to charging as expected!");
			Assert::IsTrue(chargablesource->GetConsumerLoad() == 1, L"Chargable source is not demanding maximum power to recharge!");

			Logger::WriteMessage(L"Testing depleted source behavior when no current available\n");

			manager->Evaluate(1);
			//Assert::IsTrue(chargablesource->GetConsumerLoad() == 0, L"")
			Assert::IsTrue(chargablesource->GetInputCurrent() == 0, L"Chargable source is magically getting power from somewhere!");
			Assert::IsTrue(chargablesource->GetCharge() == 0.0, L"Chargable source has somehow acumulated charge although none is available!");
			Assert::IsFalse(chargablesource->IsRunning(), L"Chargable source still thinks it is running!");
			manager->Evaluate(1000);
			Assert::IsTrue(chargablesource->GetInputCurrent() == 0, L"Chargable source is magically getting power from somewhere after large timestep!");
			Assert::IsTrue(chargablesource->GetCharge() == 0.0, L"Chargable source has somehow acumulated charge over large timestep although none is available!");
			Assert::IsFalse(chargablesource->IsParentSwitchedIn(), L"Chargable source switched to providing without respecting autoswitch threshold!");
			Assert::IsFalse(chargablesource->IsRunning(), L"Chargable source still thinks it is running!");

			Logger::WriteMessage(L"Testing charging the source\n");

			//somebody else will have to provide the power...
			source->SetParentSwitchedIn(true);
			manager->Evaluate(1000);

			Assert::IsTrue(TestUtils::IsEqual(chargablesource->GetCurrentPowerConsumption(), chargablesource->GetMaxPowerConsumption()), L"chargable source is not consuming correct amount of power!");
			Assert::IsTrue(TestUtils::IsEqual(chargablesource->GetCharge(), chargablesource->GetCurrentPowerConsumption() / 3600 * chargablesource->GetChargingEfficiency()), L"chargable source has not charged by the correct amount after one second!");
			seconds = 0;
			while (chargablesource->IsChildSwitchedIn() && seconds <= 20000)
			{
				seconds++;
				manager->Evaluate(1000);
			}

			Assert::IsTrue(seconds == 20000, L"Recharging did not take the proper amount of time!");
			Assert::IsTrue(TestUtils::IsEqual(chargablesource->GetCharge(), chargablesource->GetMaxCharge()), L"chargable source did not fully recharge!");
			Assert::IsFalse(chargablesource->IsChildSwitchedIn(), L"chargable source did not stop charging when fully recharged!");
			Assert::IsFalse(chargablesource->IsParentSwitchedIn(), L"chargable source should not start providing power unless needed!");

			//run the source half dry again.
			source->SetParentSwitchedIn(false);
			seconds = 0; //one second already passed for the last evaluation
			while (seconds < 30000)
			{
				seconds++;
				manager->Evaluate(1000);
			}

			Assert::IsTrue(TestUtils::IsEqual(chargablesource->GetCharge(), chargablesource->GetMaxCharge() / 2), L"chargable source should be halfways charged!");

			//turn of autoswitch and switch out the source
			chargablesource->SetAutoswitchEnabled(false);
			chargablesource->SetParentSwitchedIn(false);
			manager->Evaluate(1000);

			Assert::IsFalse(chargablesource->IsParentSwitchedIn(), L"Chargable source should not have started providing power with autoswitch off!");
			Assert::IsFalse(chargablesource->IsChildSwitchedIn(), L"Chargable source should not have started charging without power!");
			Assert::IsTrue(TestUtils::IsEqual(chargablesource->GetCharge(), chargablesource->GetMaxCharge() / 2), L"chargable source should not have changed charge since last evaluation!");
			Assert::IsFalse(consumer1->IsRunning(), L"consumer1 should not be receiving current!");

			//switch in other powersource
			source->SetParentSwitchedIn(true);
			manager->Evaluate(1000);

			Assert::IsFalse(chargablesource->IsChildSwitchedIn(), L"Chargable source should not have started charging with autoswitch off!");
			Assert::IsTrue(TestUtils::IsEqual(chargablesource->GetCharge(), chargablesource->GetMaxCharge() / 2), L"chargable source should not have changed charge since last evaluation!");
			Assert::IsTrue(consumer1->IsRunning(), L"consumer1 should be receiving current!");

			//force charging
			chargablesource->SetToCharging();
			manager->Evaluate(1000);
			Assert::IsTrue(chargablesource->IsChildSwitchedIn(), L"Chargable source should have started charging!");
			source->SetParentSwitchedIn(false);
			manager->Evaluate(1000);
			Assert::IsTrue(chargablesource->IsChildSwitchedIn(), L"Chargable source should have kept charger switched in!");
			Assert::IsFalse(chargablesource->IsRunning(), L"Chargable source hould not have current to charge!");

		}


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_ConverterTest)
			TEST_DESCRIPTION(L"Tests if two circuits of different voltages connected by a converter behave as intended.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_ConverterTest)
		{
			Logger::WriteMessage(L"\n\nTest: Power_ConverterTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();

			PowerSource *source = new PowerSource(110, 130, 1000, 1, 0);
			PowerBus *bus2 = new PowerBus(120, 1000, manager, 0);
			PowerConsumer *consumer3 = new PowerConsumer(15, 30, 60, 0);
			PowerConsumer *consumer4 = new PowerConsumer(15, 30, 60, 0);
			PowerConsumer *consumer5 = new PowerConsumer(15, 30, 800, 0);

			source->ConnectParentToChild(bus2);
			bus2->ConnectParentToChild(consumer5);
			bus2->ConnectParentToChild(consumer3);
			bus2->ConnectParentToChild(consumer4);

			PowerBus *bus1 = new PowerBus(26, 1000, manager, 0);
			PowerConsumer *consumer1 = new PowerConsumer(15, 30, 60, 0);
			PowerConsumer *consumer2 = new PowerConsumer(15, 30, 60, 0);

			bus1->ConnectParentToChild(consumer1);
			bus1->ConnectParentToChild(consumer2);

			PowerConverter *converter = new PowerConverter(20, 130, 1000, 0.9, 1, 0);
			bus2->ConnectParentToChild(converter);
			bus1->ConnectChildToParent(converter);
			
			Logger::WriteMessage(L"Testing with balanced circuit.\n");

			consumer1->SetConsumerLoad(1);
			consumer2->SetConsumerLoad(1);
			consumer3->SetConsumerLoad(1);
			consumer4->SetConsumerLoad(1);
			consumer5->SetConsumerLoad(0);
			
			manager->Evaluate(1);

			Logger::WriteMessage(TestUtils::Msg("current power output of source: " + to_string(source->GetCurrentPowerOutput()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(source->GetCurrentPowerOutput(), 254.1333333333333), L"Incorrect amount of power drawn from source!");
			Assert::IsTrue(TestUtils::IsEqual(converter->GetCurrentPowerOutput(), converter->GetCurrentPowerConsumption() * converter->GetConversionEfficiency()), L"Congratulations, you're violating conservation of energy!");
			Assert::IsTrue(TestUtils::IsEqual(converter->GetInputCurrent(), 1.111111111111111), L"Incorrect input current!");
			Assert::IsTrue(TestUtils::IsEqual(converter->GetOutputCurrent(), 4.615384615384615), L"Incorrect output current!");
			Assert::IsTrue(TestUtils::IsEqual(consumer1->GetConsumerLoad(), 1.0), L"Consumer1 has incorrect load!");
			Assert::IsTrue(TestUtils::IsEqual(consumer2->GetConsumerLoad(), 1.0), L"Consumer2 has incorrect load!");


			Logger::WriteMessage(L"Testing with overburdened circuit.\n");
			consumer5->SetConsumerLoad(1);
			manager->Evaluate(1);

			Assert::IsTrue(TestUtils::IsEqual(source->GetCurrentPowerOutput(), 1000), L"Incorrect amount of power drawn from source!");
			Assert::IsTrue(TestUtils::IsEqual(consumer1->GetConsumerLoad(), 1.0), L"Consumer1 has incorrect load in overburdened Circuit!");
			Assert::IsTrue(TestUtils::IsEqual(consumer2->GetConsumerLoad(), 0.2), L"Consumer2 has incorrect load in overburdened Circuit!");
			Assert::IsTrue(TestUtils::IsEqual(consumer3->GetConsumerLoad(), 1), L"Consumer3 has incorrect load in overburdened Circuit!");
			Assert::IsTrue(TestUtils::IsEqual(consumer4->GetConsumerLoad(), 1), L"Consumer4 has incorrect load in overburdened Circuit!");
			Assert::IsTrue(TestUtils::IsEqual(consumer5->GetConsumerLoad(), 1), L"Consumer5 has incorrect load in overburdened Circuit!");

			vector<PowerCircuit*> circuits;
			manager->GetPowerCircuits(circuits);
			
			Assert::IsTrue(TestUtils::IsEqual(circuits[0]->GetCircuitCurrent(), 8.333333333), L"Circuit 1 has incorrect current!");
			Assert::IsTrue(TestUtils::IsEqual(circuits[1]->GetCircuitCurrent(), 2.769230769230769), L"Circuit 1 has incorrect current!");

		}


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_MultipleConvertersTest)
			TEST_DESCRIPTION(L"Tests the behavior of an overburdened system of multiple circuits.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_MultipleConvertersTest)
		{
			Logger::WriteMessage(L"\n\nTest: Power_ConverterTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();

			PowerSource *source = new PowerSource(110, 130, 400, 1, 0);
			PowerBus *bus6 = new PowerBus(100, 1000, manager, 0);
			PowerBus *bus2 = new PowerBus(10, 1000, manager, 0);
			PowerBus *bus3 = new PowerBus(10, 1000, manager, 0);
			PowerBus *bus4 = new PowerBus(10, 1000, manager, 0);
			PowerBus *bus5 = new PowerBus(10, 1000, manager, 0);
			PowerBus *bus1 = new PowerBus(10, 1000, manager, 0);

			PowerConsumer *consumer1 = new PowerConsumer(8, 13, 100, 0);
			PowerConsumer *consumer2 = new PowerConsumer(8, 12, 100, 0);
			PowerConsumer *consumer3 = new PowerConsumer(8, 12, 100, 0);
			PowerConsumer *consumer4 = new PowerConsumer(8, 12, 100, 0);
			PowerConsumer *consumer5 = new PowerConsumer(8, 12, 100, 0);

			PowerConverter *converter1 = new PowerConverter(8, 130, 1000, 0.9, 1, 0);
			PowerConverter *converter2 = new PowerConverter(8, 130, 1000, 0.9, 1, 0);
			PowerConverter *converter3 = new PowerConverter(8, 130, 1000, 0.9, 1, 0);
			PowerConverter *converter4 = new PowerConverter(8, 130, 1000, 0.9, 1, 0);
			PowerConverter *converter5 = new PowerConverter(8, 130, 1000, 0.9, 1, 0);

			source->ConnectParentToChild(bus6);
			bus6->ConnectParentToChild(converter1);
			bus6->ConnectParentToChild(converter2);
			bus6->ConnectParentToChild(converter3);
			bus6->ConnectParentToChild(converter4);
			bus6->ConnectParentToChild(converter5);


			bus1->ConnectChildToParent(converter1);
			bus2->ConnectChildToParent(converter2);
			bus3->ConnectChildToParent(converter3);
			bus4->ConnectChildToParent(converter4);
			bus5->ConnectChildToParent(converter5);

			consumer1->ConnectChildToParent(bus1);
			consumer2->ConnectChildToParent(bus2);
			consumer3->ConnectChildToParent(bus3);
			consumer4->ConnectChildToParent(bus4);
			consumer5->ConnectChildToParent(bus5);

			consumer1->SetConsumerLoad(1);
			consumer2->SetConsumerLoad(1);
			consumer3->SetConsumerLoad(1);
			consumer4->SetConsumerLoad(1);
			consumer5->SetConsumerLoad(1);

			Logger::WriteMessage(L"Testing with insufficient standby power for last circuit\n");
			manager->Evaluate(1);

			Assert::IsTrue(TestUtils::IsEqual(source->GetCurrentPowerOutput(), 400), L"Incorrect amount of power drawn from source!"); 
			Assert::IsTrue(TestUtils::IsEqual(bus6->GetCurrent(), 4), L"Incorrect current running through bus6!");
			Assert::IsTrue(TestUtils::IsEqual(consumer1->GetConsumerLoad(), 1), L"Consumer1 has incorrect load!");
			Assert::IsTrue(consumer1->IsRunning(), L"Consumer1 should be running!");
			Assert::IsTrue(TestUtils::IsEqual(consumer2->GetConsumerLoad(), 1), L"Consumer2 has incorrect load!");
			Assert::IsTrue(consumer2->IsRunning(), L"Consumer2 should be running!");
			Assert::IsTrue(TestUtils::IsEqual(consumer3->GetConsumerLoad(), 1), L"Consumer3 has incorrect load!");
			Assert::IsTrue(consumer3->IsRunning(), L"Consumer3 should be running!");
			Assert::IsTrue(TestUtils::IsEqual(consumer4->GetConsumerLoad(), 0.591), L"Consumer4 has incorrect load!");
			Assert::IsTrue(consumer4->IsRunning(), L"Consumer4 should be running!");
			Assert::IsTrue(TestUtils::IsEqual(consumer5->GetConsumerLoad(), 0), L"Consumer5 has incorrect load!");
			Assert::IsTrue(consumer5->IsRunning(), L"Consumer5 should be running!");
		}


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_SimpleOverloadTest)
			TEST_DESCRIPTION(L"Tests if a circuit behaves correctly when there's not enough power available.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_SimpleOverloadTest)
		{
			Logger::WriteMessage(L"\n\nTest: SimpleOverloadTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();
			PowerConsumer *consumer1 = new PowerConsumer(15, 30, 60, 0);
			PowerConsumer *consumer2 = new PowerConsumer(15, 30, 60, 0);
			PowerConsumer *consumer3 = new PowerConsumer(15, 30, 60, 0);
			PowerConsumer *consumer4 = new PowerConsumer(15, 30, 60, 0);
			PowerConsumer *consumer5 = new PowerConsumer(15, 30, 60, 0);
			PowerBus *bus = new PowerBus(26, 1000, manager, 0);
			PowerSource *source = new PowerSource(15, 30, 200, 1, 0);

			source->ConnectParentToChild(bus);
			consumer1->ConnectChildToParent(bus);
			consumer2->ConnectChildToParent(bus);
			consumer3->ConnectChildToParent(bus);
			consumer4->ConnectChildToParent(bus);
			consumer5->ConnectChildToParent(bus);

			Logger::WriteMessage(L"Testing overload at full load\n");
			consumer1->SetConsumerLoad(1);
			consumer2->SetConsumerLoad(1);
			consumer3->SetConsumerLoad(1);
			consumer4->SetConsumerLoad(1);
			consumer5->SetConsumerLoad(1);
			manager->Evaluate(1);

			Assert::IsTrue(consumer1->GetConsumerLoad() == 1.0 &&
				consumer1->GetConsumerLoad() == 1.0 &&
				consumer1->GetConsumerLoad() == 1.0, L"Consumers 1 through 3 should run at maximum load!");

			Assert::IsFalse(consumer5->IsRunning(), L"Consumer5 should be completely out of power!");
			Logger::WriteMessage(TestUtils::Msg("Current consumption of consumer4: " + to_string(consumer4->GetCurrentPowerConsumption()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(consumer4->GetCurrentPowerConsumption(), 20), L"Consumer4 does not consume correct amount of power!");
			Assert::IsTrue(TestUtils::IsEqual(consumer4->GetConsumerLoad(), 0.333333333), L"Consumer4 does not have correct load!");

			vector<PowerCircuit*> circuits;
			manager->GetPowerCircuits(circuits);
			Logger::WriteMessage(TestUtils::Msg("Total circuit power: " + to_string(circuits[0]->GetCircuitCurrent() * circuits[0]->GetVoltage()) + "\n"));
			Assert::IsTrue(circuits[0]->GetCircuitCurrent() * circuits[0]->GetVoltage() == 200, L"Circuit has wrong amount of power flowing through it!");
			Assert::IsTrue(source->GetCurrentPowerOutput() == 200, L"Source has wrong power output!");
		}


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_ComplexCircuitEvaluationTest)
			TEST_DESCRIPTION(L"Tests the evaluation of a complex, but isolated circuit.")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_ComplexCircuitEvaluationTest)
		{
			/**
			The circuit tested here looks as follows
			S are sources, with id.
			BBBB are buses, with id.
			Plain numbers are consumers, the number is the id. Consumers are always written below the bus they are connected to.

			S1				  S2  S3
			|				  |   |
			BBBB1			  BBBBB2
			1 2 |			  | 5 |
				BBBB3	BBBBBB4   BBB5
				3 4 |   | 7 8     9  |
					BBBBB6			 BBBBB9
					| 6 |			 12  13
				BBBB7	BBB8
				| 10	 11
				S4

			*/

			Logger::WriteMessage(L"\n\nTest: ComplexCircuitConnectionTest\n");

			Logger::WriteMessage(L"Creating test assets\n");
			PowerCircuitManager *manager = new PowerCircuitManager();
			PowerConsumer *c1 = new PowerConsumer(8, 12, 10, 0);
			PowerConsumer *c2 = new PowerConsumer(8, 12, 20, 0);
			PowerConsumer *c3 = new PowerConsumer(8, 12, 10, 0);
			PowerConsumer *c4 = new PowerConsumer(8, 12, 30, 0);
			PowerConsumer *c5 = new PowerConsumer(8, 12, 20, 0);
			PowerConsumer *c6 = new PowerConsumer(8, 12, 50, 0);
			PowerConsumer *c7 = new PowerConsumer(8, 12, 30, 0);
			PowerConsumer *c8 = new PowerConsumer(8, 12, 50, 0);
			PowerConsumer *c9 = new PowerConsumer(8, 12, 60, 0);
			PowerConsumer *c10 = new PowerConsumer(8, 12, 30, 0);
			PowerConsumer *c11 = new PowerConsumer(8, 12, 20, 0);
			PowerConsumer *c12 = new PowerConsumer(8, 12, 10, 0);
			PowerConsumer *c13 = new PowerConsumer(8, 12, 40, 0);

			PowerSource *s1 = new PowerSource(8, 12, 200, 1, 0);
			PowerSource *s2 = new PowerSource(8, 12, 50, 4, 0);
			PowerSource *s3 = new PowerSource(8, 12, 50, 4, 0);
			PowerSource *s4 = new PowerSource(8, 12, 50, 4, 0);

			PowerBus *b1 = new PowerBus(10, 1000, manager, 0);
			PowerBus *b2 = new PowerBus(10, 1000, manager, 0);
			PowerBus *b3 = new PowerBus(10, 1000, manager, 0);
			PowerBus *b4 = new PowerBus(10, 1000, manager, 0);
			PowerBus *b5 = new PowerBus(10, 1000, manager, 0);
			PowerBus *b6 = new PowerBus(10, 1000, manager, 0);
			PowerBus *b7 = new PowerBus(10, 1000, manager, 0);
			PowerBus *b8 = new PowerBus(10, 1000, manager, 0);
			PowerBus *b9 = new PowerBus(10, 1000, manager, 0);

			Logger::WriteMessage(L"Testing circuit assembly\n");
			Assert::IsTrue(b1->CanConnectToParent(s1), L"b1 must be able to connect to s1\n");
			b1->ConnectChildToParent(s1);
			Assert::IsTrue(c1->CanConnectToParent(b1), L"c1 must be able to connect to b1\n");
			c1->ConnectChildToParent(b1);
			Assert::IsTrue(c2->CanConnectToParent(b1), L"c2 must be able to connect to b1\n");
			c2->ConnectChildToParent(b1);
			Assert::IsTrue(c3->CanConnectToParent(b3), L"c3 must be able to connect to b3\n");
			c3->ConnectChildToParent(b3);
			Assert::IsTrue(c4->CanConnectToParent(b3), L"c4 must be able to connect to b3\n");
			c4->ConnectChildToParent(b3);
			Assert::IsTrue(c5->CanConnectToParent(b2), L"c5 must be able to connect to b2\n");
			c5->ConnectChildToParent(b2);
			Assert::IsTrue(c6->CanConnectToParent(b6), L"c6 must be able to connect to b6\n");
			c6->ConnectChildToParent(b6);
			Assert::IsTrue(c7->CanConnectToParent(b4), L"c7 must be able to connect to b4\n");
			c7->ConnectChildToParent(b4);
			Assert::IsTrue(c8->CanConnectToParent(b4), L"c8 must be able to connect to b4\n");
			c8->ConnectChildToParent(b4);
			Assert::IsTrue(c9->CanConnectToParent(b5), L"c9 must be able to connect to b5\n");
			c9->ConnectChildToParent(b5);
			Assert::IsTrue(c10->CanConnectToParent(b7), L"c10 must be able to connect to b7\n");
			c10->ConnectChildToParent(b7);
			Assert::IsTrue(c11->CanConnectToParent(b8), L"c11 must be able to connect to b8\n");
			c11->ConnectChildToParent(b8);
			Assert::IsTrue(c12->CanConnectToParent(b9), L"c12 must be able to connect to b9\n");
			c12->ConnectChildToParent(b9);
			Assert::IsTrue(c13->CanConnectToParent(b9), L"c13 must be able to connect to b9\n");
			c13->ConnectChildToParent(b9);

			Assert::IsTrue(b1->CanConnectToChild(b3), L"b1 must be able to connect to b3\n");
			b1->ConnectParentToChild(b3);
			Assert::IsTrue(b2->CanConnectToChild(b5), L"b2 must be able to connect to b5\n");
			b2->ConnectParentToChild(b5);
			Assert::IsTrue(b2->CanConnectToChild(b4), L"b2 must be able to connect to b4\n");
			b2->ConnectParentToChild(b4);
			Assert::IsTrue(b3->CanConnectToChild(b6), L"b3 must be able to connect to b6\n");
			b3->ConnectParentToChild(b6);
			Assert::IsTrue(b4->CanConnectToChild(b6), L"b4 must be able to connect to b6\n");
			b4->ConnectParentToChild(b6);
			Assert::IsTrue(b5->CanConnectToChild(b9), L"b5 must be able to connect to b9\n");
			b5->ConnectParentToChild(b9);
			Assert::IsTrue(b6->CanConnectToChild(b7), L"b6 must be able to connect to b7\n");
			b6->ConnectParentToChild(b7);
			Assert::IsTrue(b6->CanConnectToChild(b8), L"b6 must be able to connect to b8\n");
			b6->ConnectParentToChild(b8);

			Assert::IsTrue(b2->CanConnectToParent(s2), L"b2 must be able to connect to s2\n");
			b2->ConnectChildToParent(s2);
			Assert::IsTrue(b2->CanConnectToParent(s3), L"b2 must be able to connect to s3\n");
			b2->ConnectChildToParent(s3);
			Assert::IsTrue(b7->CanConnectToParent(s4), L"b7 must be able to connect to s4\n");
			b7->ConnectChildToParent(s4);

			Assert::IsTrue(manager->GetSize() == 1, L"PowerCircuitManager should only have 1 circuit at this point!");


			Logger::WriteMessage(L"Testing at load 0.5\n");
			//for a first test, we'll drive all consumers at 50%
			c1->SetConsumerLoad(0.5);
			c2->SetConsumerLoad(0.5);
			c3->SetConsumerLoad(0.5);
			c4->SetConsumerLoad(0.5);
			c5->SetConsumerLoad(0.5);
			c6->SetConsumerLoad(0.5);
			c7->SetConsumerLoad(0.5);
			c8->SetConsumerLoad(0.5);
			c9->SetConsumerLoad(0.5);
			c10->SetConsumerLoad(0.5);
			c11->SetConsumerLoad(0.5);
			c12->SetConsumerLoad(0.5);
			c13->SetConsumerLoad(0.5);

			Logger::WriteMessage(L"Testing circuit evaluation\n");
		
			manager->Evaluate(1);
			vector<PowerCircuit*> circuits;
			manager->GetPowerCircuits(circuits);
			PowerCircuit *circuit = circuits[0];

			Logger::WriteMessage(TestUtils::Msg("equivalent resistance of circuit: " + to_string(circuit->GetEquivalentResistance()) + "\n"));
			Logger::WriteMessage(TestUtils::Msg("Total current in circuit: " + to_string(circuit->GetCircuitCurrent()) + "\n"));

			Assert::IsTrue(circuit->GetCircuitCurrent() == 19, L"Circuit current is incorrect!");
			Assert::IsTrue(TestUtils::IsEqual(circuit->GetEquivalentResistance(), 0.52631578947368418), L"Equivalent resistance of circuit is incorrect!");


			Logger::WriteMessage(TestUtils::Msg("Current current output of S1: " + to_string(s1->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(s1->GetOutputCurrent() == 10.857142857142858, L"Power output of S1 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current current output of S2 " + to_string(s2->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(s2->GetOutputCurrent() == 2.7142857142857144, L"Power output of S2 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current current output of S3: " + to_string(s3->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(s3->GetOutputCurrent() == 2.7142857142857144, L"Power output of S3 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current current output of S4: " + to_string(s4->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(s4->GetOutputCurrent() == 2.7142857142857144, L"Power output of S4 is incorrect!");

			
			Logger::WriteMessage(L"Testing current distribution\n");

			Logger::WriteMessage(TestUtils::Msg("Current throughput of B1 is: " + to_string(b1->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b1->GetCurrent(), 10.857142857142858), L"Current throughput of b1 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B2 is: " + to_string(b2->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b2->GetCurrent(), 6.5), L"Current throughput of b2 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B3 is: " + to_string(b3->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b3->GetCurrent(), 9.3571428571), L"Current throughput of b3 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B4 is: " + to_string(b4->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b4->GetCurrent(), 5.0714285714), L"Current throughput of b4 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B5 is: " + to_string(b5->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b5->GetCurrent(), 5.5), L"Current throughput of b5 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B6 is: " + to_string(b6->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b6->GetCurrent(), 8.5714285714), L"Current throughput of b6 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B7 is: " + to_string(b7->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b7->GetCurrent(), 2.7142857143), L"Current throughput of b7 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B8 is: " + to_string(b8->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b8->GetCurrent(), 1), L"Current throughput of b8 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B9 is: " + to_string(b9->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b9->GetCurrent(), 2.5), L"Current throughput of b9 is incorrect!");


			Logger::WriteMessage(L"Testing at load 0.9\n");
			//for a first test, we'll drive all consumers at 50%
			c1->SetConsumerLoad(0.9);
			c2->SetConsumerLoad(0.9);
			c3->SetConsumerLoad(0.9);
			c4->SetConsumerLoad(0.9);
			c5->SetConsumerLoad(0.9);
			c6->SetConsumerLoad(0.9);
			c7->SetConsumerLoad(0.9);
			c8->SetConsumerLoad(0.9);
			c9->SetConsumerLoad(0.9);
			c10->SetConsumerLoad(0.9);
			c11->SetConsumerLoad(0.9);
			c12->SetConsumerLoad(0.9);
			c13->SetConsumerLoad(0.9);

			Logger::WriteMessage(L"Testing circuit evaluation\n");

			
			manager->Evaluate(1);
			manager->GetPowerCircuits(circuits);
			circuit = circuits[0];

			Logger::WriteMessage(TestUtils::Msg("equivalent resistance of circuit: " + to_string(circuit->GetEquivalentResistance()) + "\n"));
			Logger::WriteMessage(TestUtils::Msg("Total current in circuit: " + to_string(circuit->GetCircuitCurrent()) + "\n"));

			Assert::IsTrue(circuit->GetCircuitCurrent() == 34.2, L"Circuit current is incorrect!");
			Assert::IsTrue(TestUtils::IsEqual(circuit->GetEquivalentResistance(), 0.2923976608), L"Equivalent resistance of circuit is incorrect!");


			Logger::WriteMessage(TestUtils::Msg("Current current output of S1: " + to_string(s1->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(s1->GetOutputCurrent(), 19.5428571429), L"Power output of S1 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current current output of S2 " + to_string(s2->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(s2->GetOutputCurrent(), 4.8857142857), L"Power output of S2 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current current output of S3: " + to_string(s3->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(s3->GetOutputCurrent(), 4.8857142857), L"Power output of S3 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current current output of S4: " + to_string(s4->GetOutputCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(s4->GetOutputCurrent(), 4.8857142857), L"Power output of S4 is incorrect!");


			Logger::WriteMessage(L"Testing current distribution\n");

			Logger::WriteMessage(TestUtils::Msg("Current throughput of B1 is: " + to_string(b1->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b1->GetCurrent(), 19.5428571429), L"Current throughput of b1 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B2 is: " + to_string(b2->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b2->GetCurrent(), 11.7), L"Current throughput of b2 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B3 is: " + to_string(b3->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b3->GetCurrent(), 16.8428571429), L"Current throughput of b3 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B4 is: " + to_string(b4->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b4->GetCurrent(), 9.1285714286), L"Current throughput of b4 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B5 is: " + to_string(b5->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b5->GetCurrent(), 9.9), L"Current throughput of b5 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B6 is: " + to_string(b6->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b6->GetCurrent(), 15.4285714286), L"Current throughput of b6 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B7 is: " + to_string(b7->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b7->GetCurrent(), 4.8857142857), L"Current throughput of b7 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B8 is: " + to_string(b8->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b8->GetCurrent(), 1.8), L"Current throughput of b8 is incorrect!");
			Logger::WriteMessage(TestUtils::Msg("Current throughput of B9 is: " + to_string(b9->GetCurrent()) + "\n"));
			Assert::IsTrue(TestUtils::IsEqual(b9->GetCurrent(), 4.5), L"Current throughput of b9 is incorrect!");


			Logger::WriteMessage(L"Cleaning up test assets");
		}


		BEGIN_TEST_METHOD_ATTRIBUTE(Power_EventsTest)
			TEST_DESCRIPTION(L"Tests if expected events fire at the expected time")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(Power_EventsTest)
		{
			Logger::WriteMessage(L"\n\nTest: EventsTest\n");

			int consumerSwitchInEvents = 0;
			int sourceSwitchInEvents = 0;
			int consumerSwitchOutEvents = 0;
			int sourceSwitchOutEvents = 0;
			int consumerRunningChangedEvents = 0;
			int consumerLoadChangedEvents = 0;
			int sourceLoadChangedEvents = 0;
			int chargeLowEvents = 0;
			int chargeEmptyEvents = 0;
			int busHighCurrentEvents = 0;
			int busCurrentOkEvents = 0;
			int busCurrentChangeEvents = 0;
			
			PowerCircuitManager *manager = new PowerCircuitManager();
			PowerConsumer *c1 = new PowerConsumer(8, 12, 10, 0);
			PowerSourceChargable *s1 = new PowerSourceChargable(15, 30, 10, 20, 100, 0.9, 1, 0, 0.2);
			PowerBus *b1 = new PowerBus(10, 0.9, manager, 0);

			c1->OnChildSwitchIn([&consumerSwitchInEvents](PowerChild* it) { consumerSwitchInEvents++; });
			c1->OnChildSwitchOut([&consumerSwitchOutEvents](PowerChild* it) { consumerSwitchOutEvents++; });
			c1->OnConsumerLoadChange([&consumerLoadChangedEvents](PowerConsumer* it) { consumerLoadChangedEvents++; });
			c1->OnRunningChange([&consumerRunningChangedEvents](PowerConsumer* it) { consumerRunningChangedEvents++; });


			s1->OnParentSwitchIn([&sourceSwitchInEvents](PowerParent* it) { sourceSwitchInEvents++; });
			s1->OnParentSwitchOut([&sourceSwitchOutEvents](PowerParent* it) { sourceSwitchOutEvents++; });
			s1->OnLoadChanged([&sourceLoadChangedEvents](PowerSource* it) { sourceLoadChangedEvents++; });
			s1->OnChargeLow([&chargeLowEvents](PowerSourceChargable* it) { chargeLowEvents++; });
			s1->OnChargeEmpty([&chargeEmptyEvents](PowerSourceChargable* it) { chargeEmptyEvents++; });

			b1->OnMaxCurrentHigh([&busHighCurrentEvents](PowerBus*) { busHighCurrentEvents++; });
			b1->OnMaxCurrentOk([&busCurrentOkEvents](PowerBus*) { busCurrentOkEvents++; });
			b1->OnCurrentThroughputChange([&busCurrentChangeEvents](PowerBus*) { busCurrentChangeEvents++; });

			// Test that registering switch events with a bus is not possible
			function<void(void)> busChildSwitchIn = [b1]() { b1->OnChildSwitchIn(NULL); };
			function<void(void)> busChildSwitchOut = [b1]() { b1->OnChildSwitchOut(NULL); };
			function<void(void)> busParentSwitchIn = [b1]() { b1->OnParentSwitchIn(NULL); };
			function<void(void)> busParentSwitchOut = [b1]() { b1->OnParentSwitchIn(NULL); };
			Assert::ExpectException<logic_error>(busChildSwitchIn, L"busChildSwitchIn did not throw!");
			Assert::ExpectException<logic_error>(busChildSwitchOut, L"busChildSwitchOut did not throw!");
			Assert::ExpectException<logic_error>(busParentSwitchIn, L"busParentSwitchIn did not throw!");
			Assert::ExpectException<logic_error>(busParentSwitchOut, L"busParentSwitchOut did not throw!");


			b1->ConnectChildToParent(s1);
			b1->ConnectParentToChild(c1);

			s1->SetParentSwitchedIn(false);
			s1->SetParentSwitchedIn(true);

			c1->SetChildSwitchedIn(false);
			c1->SetChildSwitchedIn(true);

			// Test that switch events were correctly triggered
			Assert::IsTrue(consumerSwitchInEvents == 1, L"Consumer1 switchedIn was called incorrect amount of times!");
			Assert::IsTrue(consumerSwitchOutEvents == 1, L"Consumer1 switchedOut was called incorrect amount of times!");

			Assert::IsTrue(sourceSwitchInEvents == 1, L"Source1 switchedIn was called incorrect amount of times!");
			Assert::IsTrue(sourceSwitchOutEvents == 1, L"Source1 switchedOut was called incorrect amount of times!");

			// drain the battery to see if battery/consumer state events are triggered
			s1->SetAutoswitchEnabled(false);
			c1->SetConsumerLoad(1);
			
			// it should take 10 hours to drain the battery. WHich means the whole circuit will collapse one millisecond later.
			for (int i = 0; i < 100; ++i)
			{
				manager->Evaluate(360000);
			}
			manager->Evaluate(1); 
			manager->Evaluate(1);
			manager->Evaluate(1);

			Assert::IsTrue(consumerLoadChangedEvents == 1, L"Consumer1 OnLoadChange was called incorrect amount of times!");
			Assert::IsTrue(consumerRunningChangedEvents == 1, L"Consumer1 OnRunningChange was called incorrect amount of times!");
			
			Assert::IsTrue(sourceLoadChangedEvents == 1, L"Source OnLoadChange was called incorrect amount of times!");
			Assert::IsTrue(chargeLowEvents == 1, L"Source OnChargeLow was called incorrect amount of times!");
			Assert::IsTrue(chargeEmptyEvents == 1, L"Source OnChargeEmpty was called incorrect amount of times!");
			Assert::IsTrue(sourceSwitchOutEvents == 2, L"Source OnSwitchOut was called incorrect amount of times!");

			Assert::IsTrue(busHighCurrentEvents == 1, L"bus1 OnHighCurrent was called incorrect amount of times!");
			Assert::IsTrue(busCurrentOkEvents == 1, L"bus1 OnCurrentOk was called incorrect amount of times!");
			Assert::IsTrue(busCurrentChangeEvents == 2, L"bus1 OnCurrentOk was called incorrect amount of times!");

			delete manager;
			delete b1;
			delete s1;
			delete c1;
		}
	};
}