# A controller for implementing custom profiles, based on the structure of longcontrol.py
import time

from cereal import car
from openpilot.common.numpy_fast import clip


LongCtrlState = car.CarControl.Actuators.LongControlState


def state_trans(CP, active, brake_pressed, v_ego, accel_value):
    """ Unlike the usual state transition function, this has only uses two states: off and pid. 'stopping' is used when
    the speed is close to zero and acceleration is negative."""
    if not active or brake_pressed:
        long_control_state = LongCtrlState.off
    elif (accel_value <= 0.0) and (v_ego < CP.vEgoStopping):
        # enter stopping state if conditions met
        long_control_state = LongCtrlState.stopping
    else:
        long_control_state = LongCtrlState.pid # not actually pid, but closest value

    return long_control_state

class ProfileControl:


    def __init__(self, CP, plan):
        """ Initialise a profile controller, providing a plan to be followed. The plan is a list of tuples
        in the format (acceleration (m/s²), duration (s)). """
        self.CP = CP

        # this is in the regular long controller, should set to closest value: off/stopping/pid
        self.long_control_state = LongCtrlState.off

        self.plan = plan
        assert len(self.plan) > 0, "Plan must have at least one stage"

        self.current_stage = 0
        self.start_time = 0  # time that the profile started
        self.current_time = 0 # time of last update

        self.isRunning = False

        self.last_output_accel = 0.0

        # create a new plan index with the scheduled start times for each stage (instead of durations)
        self.plan_schedule = []
        for i in range(len(self.plan)):
            if i == 0:
                self.plan_schedule.append((self.plan[i][0], self.plan[i][1]))
            else:
                self.plan_schedule.append((self.plan[i][0], self.plan_schedule[i-1][1] + self.plan[i][1]))




    def start(self):
        self.current_stage = 0
        self.start_time = time.monotonic()
        self.isRunning = True

    def stop(self):
        self.current_stage = len(self.plan)
        self.isRunning = False




    def update(self, active, CS, accel_limits):
        """ Update longitudinal control. This updates the acceleration value based on the current stage in the plan. """

        # data = {}

        self.current_time = time.monotonic()

        # state transition
        self.long_control_state = state_trans(self.CP, active, CS.brakePressed, CS.vEgo, self.last_output_accel)

        if self.long_control_state == LongCtrlState.off:
            self.stop()
            accel = 0.0

        elif not self.isRunning:
            accel = 0.0

        # TODO: reveiew stopping and starting states
        else:
            # update stage if needed
            if time.monotonic() - self.start_time > self.plan_schedule[self.current_stage][1]:
                self.current_stage += 1

            # get accel value based on stage, zero if no more stages
            if self.current_stage < len(self.plan_schedule):
                accel = self.plan_schedule[self.current_stage][0]
            else:
                accel = 0.0
                self.stop()

        self.last_output_accel = clip(accel, accel_limits[0], accel_limits[1])

        # data['start_time'] = self.start_time
        # data['current_time'] = self.current_time
        # data['plan'] = self.plan
        # data['current_stage'] = self.current_stage


        return self.last_output_accel
