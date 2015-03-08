<?php
/**
 * @file
 * role_export.features.user_role.inc
 */

/**
 * Implements hook_user_default_roles().
 */
function role_export_user_default_roles() {
  $roles = array();

  // Exported role: administrator.
  $roles['administrator'] = array(
    'name' => 'administrator',
    'weight' => 8,
  );

  // Exported role: church friend.
  $roles['church friend'] = array(
    'name' => 'church friend',
    'weight' => 13,
  );

  // Exported role: church member.
  $roles['church member'] = array(
    'name' => 'church member',
    'weight' => 14,
  );

  // Exported role: editor.
  $roles['editor'] = array(
    'name' => 'editor',
    'weight' => 12,
  );

  // Exported role: group editor.
  $roles['group editor'] = array(
    'name' => 'group editor',
    'weight' => 15,
  );

  // Exported role: group leader.
  $roles['group leader'] = array(
    'name' => 'group leader',
    'weight' => 16,
  );

  // Exported role: group member.
  $roles['group member'] = array(
    'name' => 'group member',
    'weight' => 17,
  );

  // Exported role: noncom group member.
  $roles['noncom group member'] = array(
    'name' => 'noncom group member',
    'weight' => 18,
  );

  // Exported role: office admin.
  $roles['office admin'] = array(
    'name' => 'office admin',
    'weight' => 19,
  );

  // Exported role: office staff.
  $roles['office staff'] = array(
    'name' => 'office staff',
    'weight' => 20,
  );

  // Exported role: webmaster.
  $roles['webmaster'] = array(
    'name' => 'webmaster',
    'weight' => 21,
  );

  return $roles;
}
