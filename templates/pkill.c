

#process "pkill"

// This process has objects with the following auto classes:
class auto_stability;
class auto_att_fwd;
class auto_move;
class auto_retro;
class auto_att_left;
class auto_att_right;

// The following auto classes are not currently used by any objects:
class auto_att_main;
class auto_att_back;
class auto_att_spike;
class auto_harvest;
class auto_allocate;

core_hex_C, 6144, 
  {object_downlink, 0, 
    {component_tri, // component 1
      {object_uplink, 0},
      {object_stability:auto_stability, 0},
      {object_downlink, 256, 
        {component_cap, // component 2
          {object_uplink, 0},
          {object_downlink, 384, 
            {component_tri, // component 3
              {object_pulse_xl:auto_att_fwd, 0},
              {object_uplink, 0},
              {object_interface, 0},
            }
          },
          {object_downlink, 256, 
            {component_long5, // component 4
              {object_uplink, 0},
              {object_move:auto_move, 792},
              {object_move:auto_move, 1487},
              {object_move:auto_move, -529},
              {object_interface, 0},
            }
          },
          {object_downlink, 256, 
            {component_tri, // component 5
              {object_uplink, 0},
              {object_move:auto_move:auto_retro, 1411},
              {object_move:auto_move, 1979},
            }
          },
        }
      },
    }
  },
  {object_downlink, -64, 
    {component_prong, // component 6
      {object_pulse_xl:auto_att_fwd, 0},
      {object_stability:auto_stability, 0},
      {object_pulse_xl:auto_att_left, 0},
      {object_uplink, 0},
    }
  },
  {object_downlink, 64, 
    {component_prong, // component 7
      {object_stability:auto_stability, 0},
      {object_pulse_xl:auto_att_fwd, 0},
      {object_uplink, 0},
      {object_pulse_xl:auto_att_right, 0},
    }
  },
  {object_downlink, 0, 
    {component_tri, // component 8
      {object_uplink, 0},
      {object_downlink, -256, 
        {component_cap, // component 9
          {object_downlink, -256, 
            {component_tri, // component 10
              {object_uplink, 0},
              {object_move:auto_move, -1979},
              {object_move:auto_move:auto_retro, -1411},
            }
          },
          {object_downlink, -256, 
            {component_long5, // component 11
              {object_uplink, 0},
              {object_move:auto_move, -1487},
              {object_move:auto_move, -792},
              {object_interface, 0},
              {object_move:auto_move, 529},
            }
          },
          {object_downlink, -384, 
            {component_tri, // component 12
              {object_pulse_xl:auto_att_fwd, 0},
              {object_interface, 0},
              {object_uplink, 0},
            }
          },
          {object_uplink, 0},
        }
      },
      {object_stability:auto_stability, 0},
    }
  },
  {object_downlink, 656, 
    {component_tri, // component 13
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, 0},
    }
  },
  {object_downlink, -656, 
    {component_tri, // component 14
      {object_uplink, 0},
      {object_interface, 0},
      {object_interface, 0},
    }
  }
#code




// Process AI modes (these reflect the capabilities of the process)
enum
{
  MODE_IDLE, // process isn't doing anything ongoing
  MODE_MOVE, // process is moving to target_x, target_y
  MODE_MOVE_ATTACK, // process is moving, but will attack anything it finds along the way
  MODE_ATTACK, // process is attacking a target it was commanded to attack
  MODE_ATTACK_FOUND, // process is attacking a target it found itself
  MODES
};

// Commands that the user may give the process
// (these are fixed and should not be changed, although not all processes accept all commands)
enum
{
  COM_NONE, // no command
  COM_LOCATION, // user has right-clicked somewhere on the display or map
  COM_TARGET, // user has right-clicked on an enemy process
  COM_FRIEND, // user has right-clicked on a friendly process
  COM_DATA_WELL // user has right-clicked on a data well
};

// Targetting information
// Targetting memory allows processes to track targets (enemy or friend)
// The following enums are used as indices in the process' targetting memory
enum
{
  TARGET_PARENT, // a newly built process starts with its builder in address 0
  TARGET_MAIN, // main target
  TARGET_FRONT, // target of directional forward attack
  TARGET_LEFT, // target of directional left attack
  TARGET_RIGHT, // target of directional right attack
  TARGET_GUARD, // target of guard command
};


// Variable declaration and initialisation
//  (note that declaration and initialisation cannot be combined)
//  (also, variables retain their values between execution cycles)
int angle; // direction process is pointing
 // angles are in integer degrees from 0 to 8192, with 0 being right,
 // 2048 down, 4096 left and 6144 (or -2048) up.

int mode; // what is the process doing? (should be one of the MODE enums)
int saved_mode; // save the process' mode while it's attacking something it found

int move_x, move_y; // destination
int target_x, target_y; // location of target (to attack, follow etc)

int front_attack_primary; // is set to 1 if forward directional attack objects are attacking
 // the primary target (e.g. target selected by command). Set to 0 if the objects are available for autonomous fire.
int target_component; // target component for an attack command (allows user to
 // target specific components)

int scan_result; // used to hold the results of a scan of nearby processes

int initialised; // set to 1 after initialisation code below run the first time

if (!initialised)
{
  angle = get_core_angle();
   // initialisation code goes here (not all autocoded processes have initialisation code)
  initialised = 1;
  attack_mode(0); // attack objects (if present) will all fire together
  // move the process forward a bit to stop it obstructing the next process to be built
  mode = MODE_MOVE;
  move_x = get_core_x() + cos(angle, 300);
  move_y = get_core_y() + sin(angle, 300);
  auto_stability.set_stability(1); // activate stability objects
}

if (check_selected_single()) // returns 1 if the user has selected this process (and no other processes)
{
  set_debug_mode(1); // 1 means errors for this process will be printed to the console. Resets to 0 each cycle
}


// Accept commands from user
if (check_new_command() == 1) // returns 1 if a command has been given
{
  switch(get_command_type()) // get_command_type() returns the type of command given
  {
    case COM_LOCATION:
    case COM_FRIEND:
    case COM_DATA_WELL: // this process can't harvest, so treat data well commands as location commands
      move_x = get_command_x(); // get_command_x() and ...y() return the target location of the command
      move_y = get_command_y();
      if (get_command_ctrl() == 0) // returns 1 if control was pressed when the command was given
      {
        mode = MODE_MOVE;
      }
        else
        {
          mode = MODE_MOVE_ATTACK; // will attack anything found along the way
        }
      break;
    
    case COM_TARGET:
      get_command_target(TARGET_MAIN); // writes the target of the command to address TARGET_MAIN in targetting memory
       // (targetting memory stores the target and allows the process to examine it if it's in scanning range
       //  of any friendly process)
      mode = MODE_ATTACK;
      target_x = get_command_x();
      target_y = get_command_y();
      target_component = get_command_target_component(); // allows user to target a specific component
      break;
    
    default:
      break;
  
  } // end of command type switch
} // end of new command code


front_attack_primary = 0; // this will be set to 1 if front attack is attacking the main target


// What the process does next depends on its current mode
switch(mode)
{
  
  case MODE_IDLE:
    auto_move.set_power(0); // turn off all objects in the move class
    // now check for nearby hostile processes
    scan_result = scan_for_threat(0, 0, TARGET_MAIN); // scan_for_threat finds the hostile process nearest to the scan centre,
     // and saves it in the process' targetting memory.
     // (parameters are: (x offset of scan centre from core, y offset, targetting memory address))
    if (scan_result != 0)
    {
      mode = MODE_ATTACK_FOUND; // later code means that process will attack target in targetting memory 0
      target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
      target_y = process[TARGET_MAIN].get_core_y();
      target_component = 0; // attack the core
      saved_mode = MODE_IDLE; // when leaving MODE_ATTACK_FOUND, will return to this mode
    }
    break;
  
  case MODE_MOVE_ATTACK:
  // check for nearby hostile processes
    scan_result = scan_for_threat(0, 0, TARGET_MAIN); // scan_for_threat finds the hostile process nearest to the scan centre,
     // and saves it in the process' targetting memory.
     // (parameters are: (x offset of scan centre from core, y offset, targetting memory address))
    if (scan_result != 0)
    {
      mode = MODE_ATTACK_FOUND; // later code means that process will attack target in targetting memory 0
      target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
      target_y = process[TARGET_MAIN].get_core_y();
      target_component = 0; // attack the core
      saved_mode = MODE_MOVE_ATTACK; // when leaving MODE_ATTACK_FOUND, will return to this mode
      break;
    }
  // fall through to MODE_MOVE case...
  
  case MODE_MOVE:
  // stop moving when within 255 pixels of target (can change to higher or lower values if needed)
    if (distance_from_xy_less(move_x, move_y, 255))
    {
      clear_command(); // cancels the current command. If there's a queue of commands (e.g. shift-move waypoints)
       //  this moves the queue forward so that check_new_command() will return 1 next cycle.
      mode = MODE_IDLE;
    }
      else
        auto_move.move_to(move_x, move_y); // calls move_to for all objects in the move class
    break;
  
  case MODE_ATTACK_FOUND:
  // Attack target as long as it's visible.
  // If target lost or destroyed, go back to previous action.
    // Now see whether the commanded target is visible:
    //  (targets are visible if within scanning range of any friendly process)
    if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist
    {
      auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class
      if (distance_from_xy_less(target_x, target_y, 600))
      {
        // we should be able to see the target now, so it's either been destroyed
        // or gone out of range.
        mode = saved_mode; // the process goes back to what it was doing
        break;
      }
    }
      else
      {
        target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
        target_y = process[TARGET_MAIN].get_core_y();
        auto_move.approach_target(TARGET_MAIN,target_component,700);
         // approach_target() approaches a target to within a certain distance (700 in this case).
         //  if the process has retro move objects it will use them to maintain the distance.
         // Parameters are:
         //  - target's address in targetting memory
         //  - component of target process to attack
         //  - stand-off distance (in pixels)
        if (distance_from_xy_less(target_x, target_y, 1000))
        {
          auto_att_fwd.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the forward directional attack class.
           // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.
           // target_component is the component to fire at (can be set by user's command; defaults to 0 (attack core)).
           // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.
          front_attack_primary = 1; // this means the forward directional attack class will not try to find its own target.
        }
      }
    break;
  
  case MODE_ATTACK:
  // Attack target identified by user command
    // Now see whether the commanded target is visible:
    //  (targets are visible if within scanning range of any friendly process)
    if (!process[TARGET_MAIN].visible()) // returns zero if not target visible or doesn't exist
    {
      auto_move.move_to(target_x, target_y); // calls move_to for all objects in the move class
      if (distance_from_xy_less(target_x, target_y, 600))
      {
        // we should be able to see the target now, so it's either been destroyed
        // or gone out of range.
        mode = saved_mode; // the process goes back to what it was doing
        break;
      }
    }
      else
      {
        target_x = process[TARGET_MAIN].get_core_x(); // calls get_core_x() on the process in targetting memory address TARGET_MAIN
        target_y = process[TARGET_MAIN].get_core_y();
        auto_move.approach_target(TARGET_MAIN,target_component,700);
         // approach_target() approaches a target to within a certain distance (700 in this case).
         //  if the process has retro move objects it will use them to maintain the distance.
         // Parameters are:
         //  - target's address in targetting memory
         //  - component of target process to attack
         //  - stand-off distance (in pixels)
        if (distance_from_xy_less(target_x, target_y, 1000))
        {
          auto_att_fwd.fire_at(TARGET_MAIN,target_component); // Calls fire_at() on all objects in the forward directional attack class.
           // TARGET_MAIN indicates that the target is in targetting memory address TARGET_MAIN.
           // target_component is the component to fire at (can be set by user's command; defaults to 0 (attack core)).
           // fire_at() rotates directional attack objects towards the target (with basic leading) and fires.
          front_attack_primary = 1; // this means the forward directional attack class will not try to find its own target.
        }
      }
    break;

} // end of mode switch

if (front_attack_primary == 0) // is 1 if the forward attack objects are attacking the main target
{
  auto_att_fwd.attack_scan(0, 400, TARGET_FRONT);
}

auto_att_left.attack_scan(-2048, 400, TARGET_LEFT);

auto_att_right.attack_scan(2048, 400, TARGET_RIGHT);

charge_interface_max(); // charges the process' interface. Since the interface is shared across all
 // components with interface objects, this call is not specific to any object or class.
 // charge_interface_max() charges the interface using as much power as possible
 // (the charge rate is determined by the maximum interface strength).

exit; // stops execution, until the next cycle
