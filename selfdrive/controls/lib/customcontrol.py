# A controller for receiving target acceleration values from MABXII over CAN
import time

from cereal import car
from openpilot.common.numpy_fast import clip
import cereal.messaging as messaging

LongCtrlState = car.CarControl.Actuators.LongControlState


def state_trans(CP, active, brake_pressed, v_ego, accel_value):
    """ Unlike the usual state transition function, this has only uses two states: off and pid. 'stopping' is used when
    the speed is close to zero and acceleration is negative."""
    if not active:
        long_control_state = LongCtrlState.off
    elif (accel_value <= 0.0) and (v_ego < CP.vEgoStopping):
        # enter stopping state if conditions met
        long_control_state = LongCtrlState.stopping
    else:
        long_control_state = LongCtrlState.pid  # not actually pid, but closest value

    return long_control_state


class CustomControl:
    def __init__(self, CP, plan=None):
        """ Initialize a controller for MABXII integration.
        The plan parameter is kept for backward compatibility but is not used. """
        self.CP = CP

        # Control state: off/stopping/pid
        self.long_control_state = LongCtrlState.off

        # Timing
        self.current_stage = 0
        self.start_time = 0  # time that control started
        self.current_time = 0  # time of last update

        # Status flags
        self.isRunning = False
        self.mabxii_active = False

        # Acceleration values
        self.target_accel = 0.0
        self.last_output_accel = 0.0

        # History for UI display
        self.history = []

        # Subscribe to CAN messages
        self.can = messaging.sub_sock('can', timeout=0)

        # Track MABXII message timing
        self.last_mabxii_msg_time = 0
        self.mabxii_timeout = 0.5  # seconds

    def start(self):
        """ Start the controller """
        self.current_stage = 0
        self.start_time = time.monotonic()
        self.isRunning = True
        self.history = []

    def stop(self):
        """ Stop the controller """
        self.isRunning = False
        self.start_time = 0.0
        self.mabxii_active = False

    def read_mabxii_can(self):
        """ Read acceleration values from MABXII over CAN bus
        Returns True if a valid message was received """
        # Get all new CAN messages
        can_msgs = messaging.drain_sock(self.can)

        for msg in can_msgs:
            if msg.which() == 'can':
                for c in msg.can:
                    # MABXII message with ID 0x029 on bus 1 (A-CAN)
                    if c.address == 0x029 and c.src == 1:
                        # Extract target acceleration from CAN message
                        # Assuming acceleration is stored as a signed 16-bit value in the first 2 bytes
                        # scaled by 100 (e.g., 123 = 1.23 m/s²)
                        accel_bytes = bytes(c.dat[0:2])
                        accel_raw = int.from_bytes(accel_bytes, byteorder='little', signed=True)
                        self.target_accel = accel_raw / 100.0

                        # Update timing information
                        self.last_mabxii_msg_time = time.monotonic()
                        self.mabxii_active = True
                        return True

        # Check for MABXII timeout
        if self.mabxii_active and (time.monotonic() - self.last_mabxii_msg_time > self.mabxii_timeout):
            self.mabxii_active = False
            self.target_accel = 0.0

        return False

    def update(self, active, CS, accel_limits):
        """ Update longitudinal control based on MABXII input """
        self.current_time = time.monotonic()

        # State transition logic
        self.long_control_state = state_trans(self.CP, active, CS.brakePressed, CS.vEgo, self.last_output_accel)

        if self.long_control_state == LongCtrlState.off:
            self.stop()
            accel = 0.0
        elif not self.isRunning:
            accel = 0.0
        else:
            # Read target acceleration from MABXII
            self.read_mabxii_can()

            # Use the target acceleration if MABXII is active
            if self.mabxii_active:
                accel = self.target_accel

                # Record in history for UI display
                elapsed = self.current_time - self.start_time
                if len(self.history) < 20:  # Limit history size
                    self.history.append((round(elapsed, 4), accel))
            else:
                accel = 0.0

        # Apply acceleration limits and return
        self.last_output_accel = clip(accel, accel_limits[0], accel_limits[1])
        return self.last_output_accel
