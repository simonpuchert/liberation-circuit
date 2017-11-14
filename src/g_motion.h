
#ifndef H_G_MOTION
#define H_G_MOTION

//void init_drag_table(void);

void run_motion(void);

void add_proc_to_blocklist(struct proc_struct* pr);
//int check_proc_point_collision(struct proc_struct* pr, al_fixed x, al_fixed y);

int check_notional_block_collision_multi(int notional_shape, al_fixed notional_x, al_fixed notional_y, al_fixed notional_angle, int notional_proc_mobile, int notional_proc_player_index, struct core_struct** collision_core);
void apply_impulse_to_proc(struct proc_struct* pr, cart lever, cart impulse);
void apply_impulse_to_group(struct core_struct* group_core, cart lever, cart impulse);

int check_notional_solid_block_collision(int notional_shape, al_fixed notional_x, al_fixed notional_y, al_fixed notional_angle);

#include "g_shapes.h"

int check_nshape_nshape_collision(struct nshape_struct* nshape1, int nshape2_index, al_fixed sh1_x, al_fixed sh1_y, al_fixed sh1_angle, al_fixed sh2_x, al_fixed sh2_y, al_fixed sh2_angle);

#endif


