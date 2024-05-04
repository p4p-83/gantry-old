#ifndef STEPPER_CONTROL_H_
#define STEPPER_CONTROL_H_

void init_steppers();

void disable_steppers();

void set_target(float x, float y, float z);

long getMaxSpeed();
long calculate_feedrate_delay(float feedrate);
void dda_move(long micro_delay);
void calculate_deltas();
void goto_machine_zero();
void set_position(float x, float y, float z);
void move_to_max(int limiter_pin, int stepper_pin, int stepper_dir_pin, int dir);
bool can_step(byte min_pin, byte max_pin, long current, long target, byte direction);
void do_step(byte pinA, byte pinB, byte dir);
bool read_switch(byte pin);

#endif
