// Dummy implementation just kills plane after 60 seconds.

bool wasHigh = false;
bool onFinal = false;
uint32_t finalPhaseTimeout;

// Check if plane lost. Defined as flying high, comoing low and still having
// power after 10 minutes at low altitude.
void check_crashed() {

	// If absolute alt. exceeds 1000m ~3kfeet then we consider ourselves high.
	if (current_loc.alt > 1000*100L) wasHigh = true;

	// Cancel everything if control_mode is not AUTO. That indicates that the pilot is
	// still in command.
	if (control_mode != AUTO && control_mode != RTL) {
		onFinal = false;
		return;
	}
	
	// If we are below 500m start a timer to kill servos.
	if (wasHigh) {
		if (!onFinal && current_loc.alt < 500*100L) {
			onFinal = true;
			// after 5 minutes of this event, we kill the servos.
			finalPhaseTimeout = millis() + 10L * 60 * 1000;
		} else onFinal = false;
	}
	
	if (onFinal && millis() > finalPhaseTimeout) {
		isCrashed = true;
		// disable all logging
		g.log_bitmask.set(0);
	}
}

bool check_rcin_killswitch() {
    int16_t pwmin = g.rc_7.radio_in;
    return (pwmin < 1500);
}

void check_killRCOut() {
	check_crashed();
	bool killRC = check_rcin_killswitch();
	servoOutEnabled = !killRC;
}
