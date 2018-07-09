#pragma once

class PowerConsumer;
class PowerCircuit;
class PowerCircuitManager;

class PowerBus : public PowerChild, public PowerParent
{
	friend class PowerCircuitManager;
public:
	/**
	 * \param voltage The voltage at which this bus is intended to operate.
	 * \param maxamps The amount of amperes this bus is designed to tolerate (regular load)
	 * \param PowerCircuitManager THe circuit manager this bus belongs to.
	 * \param location_id The identifier of the objects location. Unless both objects are global,
	 *	relationships can only be formed with objects in the same location.
	 * \note Buses are always global, i.e. any other global object can form a relationship with any bus.
	 */
	PowerBus(double voltage, double maxamps, PowerCircuitManager *circuitmanager, unsigned int location_id);
	virtual ~PowerBus();

	/**
	 * \return The equivalent resistance of all consumers of this bus at this point in time, in Ohm.
	 * \note Does not take into account other buses that might be connected to this bus.
	 */
	double GetEquivalentResistance();

	/**
	 * \return The total current flowing through this bus at this moment in time, in Amperes.
	 */
	double GetCurrent();

	/**
	 * \return The maximum current this bus is designed for.
	 */
	double GetMaxCurrent();

	/**
	 * \brief Sets the current flowing through this bus at this moment, in Amperes.
	 * \note ONLY call from the containing circuit!
	 */
	void SetCurrent(double amps);

	/**
	 * Sets the maximum current for this bus, in Amperes.
	 * Use to simulate degradation.
	 */
	void SetMaxCurrent(double amps);

	/**
	 * \brief Bus will attempt to reduce its current flow by shutting down consumers.
	 * \param missing_current The amount of current the bus should reduce in amps
	 * \return How much of the missing current is "left". Can be negative if current was reduced by more than was asked!
	 */
	double ReduceCurrentFlow(double missing_current);

	/**
	 * \brief Tells the bus to calculate the total current running through it.
	 * This is the last operation in circuit evaluation to be called. Should not ever be called
	 * under any other circumstances.
	 * \param deltatime Time passed since the last evaluation, in miliseconds.
	 */
	void CalculateTotalCurrentFlow(double deltatime);

	/**
	 * \brief Lets the bus delete all its feeding subcircuits and reconstruct them anew.
	 * This is a relatively expensive operation, but is neccessary to perform when the structure
	 * of a circuit changes.
	 * \todo This could be optimised by introducing structural state observers, so only affected
	 *	subcircuits get rebuilt. It would increase complexity quite a bit, but is a feasible option
	 *	if circuit building turns out to take too long in the sim.
	 */
	void RebuildFeedingSubcircuits();

	/**
	 * \return The PowerCircuitManager this bus is controlled by.
	 */
	PowerCircuitManager *GetCircuitManager();

	//PowerParent implementation
	virtual void Evaluate(double deltatime);

	virtual bool CanConnectToChild(PowerChild *child, bool bidirectional = true);

	virtual void ConnectParentToChild(PowerChild *child, bool bidirectional = true);

	virtual void DisconnectParentFromChild(PowerChild *child, bool bidirectional = true);

	//PowerChild implementation
	virtual void ConnectChildToParent(PowerParent *parent, bool bidirectional = true);

	virtual void DisconnectChildFromParent(PowerParent *parent, bool bidirectional = true);
	
	virtual bool CanConnectToParent(PowerParent *parent, bool bidirectional = true);

	virtual double GetChildResistance();

	virtual unsigned int GetLocationId();

	virtual bool IsGlobal();

	/**
	* \brief register lambda that fires when the current throughput of this bus changes.
	* \param lambda Lambda function that receives this as an argument.
	*/
	virtual void OnCurrentThroughputChange(function<void(PowerBus*)> lambda) { currentThroughputChanged = lambda; };

	/**
	* \brief register lambda that fires when the maximum current of this bus is breached.
	* \param lambda Lambda function that receives this as an argument.
	*/
	virtual void OnMaxCurrentHigh(function<void(PowerBus*)> lambda) { maxCurrentHigh = lambda; };

	/**
	* \brief register lambda that fires when the load of the bus falls into save range again.
	* \param lambda Lambda function that receives this as an argument.
	*/
	virtual void OnMaxCurrentOk(function<void(PowerBus*)> lambda) { maxCurrentOk = lambda; };

	/**
	 * \brief Buses cannot be actively switched, therefore it is impossible to register switch events for them!
	 *\throws runtime_error
	 */
	virtual void OnChildSwitchIn(function<void(PowerChild*)> lambda) { throw logic_error("Bus cannot be switched, do not register event!"); };

	/**
	* \brief Buses cannot be actively switched, therefore it is impossible to register switch events for them!
	*\throws runtime_error
	*/
	virtual void OnChildSwitchOut(function<void(PowerChild*)> lambda) { throw logic_error("Bus cannot be switched, do not register event!"); };

	/**
	 * \brief Buses cannot be actively switched, therefore it is impossible to register switch events for them!
 	 *\throws runtime_error
	 */
	virtual void OnParentSwitchIn(function<void(PowerParent*)> lambda) { throw logic_error("Bus cannot be switched, do not register event!"); };

	/**
	* \brief Buses cannot be actively switched, therefore it is impossible to register switch events for them!
	*\throws runtime_error
	*/
	virtual void OnParentSwitchOut(function<void(PowerParent*)> lambda) { throw logic_error("Bus cannot be switched, do not register event!"); };

protected:

	double maxcurrent = -1;
	double equivalent_resistance = -1;
	double throughcurrent = -1;

	PowerCircuitManager *circuitmanager = NULL;
	vector<PowerSubCircuit*> feeding_subcircuits;				//!< The subcircuits feeding current to this bus.
	
	function<void(PowerBus*)> currentThroughputChanged = NULL;
	function<void(PowerBus*)> maxCurrentHigh = NULL;
	function<void(PowerBus*)> maxCurrentOk = NULL;

private:
	unsigned int locationid = 0;
};

