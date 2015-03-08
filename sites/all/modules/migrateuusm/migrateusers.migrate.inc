<?php 


class UsersMigration extends Migration {
  public function __construct() {
    parent::__construct(MigrateGroup::getInstance('UUSMMigrate'));
    ini_set('auto_detect_line_endings', TRUE);
    // Select fields from the Drupal 6 user table.
    $this->description = t("import users and roles");
    $options = array('track_changes' => 1);
    $query = Database::getConnection('default','for_migration')->select('users', 'u')->fields('u',array(
        'uid', 
        'name', 
        'pass', 
        'mail', 
        'created', 
        'access',
        'login', 
        'status', 
        'picture', 
        'init'));
    // This is supposed to get roles not sure how.
    $source_fields = array(
      'uid' => t('User ID'),
      'roles' => t('The set of roles assigned to a user.'),
    );
    // Set source and destination.
    $this->source = new MigrateSourceSQL($query, $source_fields);
    $this->destination = new MigrateDestinationUser(array('md5_passwords' => TRUE));
    // Set up database mapping.
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'uid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D6 Unique User ID',
          'alias' => 'u',
        )
      ),
      MigrateDestinationUser::getKeySchema()
    );
    // Add simple field mappings
    $this->addFieldMapping('is_new')
      ->defaultValue(TRUE);
    $this->addFieldMapping('uid', 'id_to_preserve');
    $this->addFieldMapping('name', 'name')->dedupe('users', 'name');
    $this->addFieldMapping('pass', 'pass');
    $this->addFieldMapping('mail', 'mail')->dedupe('users', 'mail');
    $this->addFieldMapping('language')->defaultValue('');
    $this->addFieldMapping('theme')->defaultValue('');
    $this->addFieldMapping('signature')->defaultValue('');
    $this->addFieldMapping('signature_format')->defaultValue('filtered_html');
    $this->addFieldMapping('created', 'created');
    $this->addFieldMapping('access', 'access');
    $this->addFieldMapping('login', 'login');
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('picture')->defaultValue(0);
    $this->addFieldMapping('init', 'init');
    $this->addFieldMapping('timezone')->defaultValue(NULL);
    $this->addFieldMapping('roles', 'roles');
  }
  // Set up Roles.
  // massage the user roles before saving
  public function prepareRow($current_row) {
    $source_id = $current_row->uid;
    $query = Database::getConnection('default','for_migration')
      ->select('users_roles', 'r')
      ->fields('r', array('uid', 'rid'))
      ->havingCondition('r.uid', $source_id, '=');
    $results = $query->execute();
    // add the anonymous role for everyone
    $roles = array('1' => '1');
    foreach ($results as $row) {
      // adjust the old to new role id's
      // Webmaster (rid 8) needs role id 3 to match admin because
      //the current administrator role is 3.
      if ($row->rid == 8) {
        $roles['3'] = '3';
      }
      elseif ($row->rid == 10) {
        // group editor from 10 to 4 to match editor role in D7 installation
        $roles['4'] = '4';
      }
      elseif ($row->rid == 3) {
        // church member from 3 to 8
        $roles['8'] = '8';
      }
      elseif ($row->rid == 4) {
        // group leader 4 to 10
        $roles['10'] = '10';
      }
      else {
        // The rest are the same, mostly because there are 7 more roles from D6 that go into D7
        $roles[$row->rid] = $row->rid;
      }
    }
    $current_row->roles = $roles;
    return TRUE;
  }
} // close the ourUsersMigration class