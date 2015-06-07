<?php
/* Mostly taken from http://dtek.net/blog/drupal-6-to-drupal-7-via-migrate-2 */
class ChurchUsersMigration extends Migration {
  public function __construct($arguments)
  {
    parent::__construct($arguments);

    // Avoid known line ending issue: "Invalid data value" at drupal.org/node/1152158#InvalidDataValue
    ini_set('auto_detect_line_endings', TRUE);

    // Tell it that 'drush mi Roles' has to come first
    $this->dependencies = array('Roles');

    $this->description = t("import users");
    $options = array('track_changes' => 1);

    // some field definitions; roles will be added later in prepareRow()
    $source_fields = array(
      'uid' => t('User ID'),
      'roles' => t('The set of roles assigned to a user.'),
    );

    $query = Database::getConnection('default','for_migration')->select('users', 'u');
    // only import active users from the old db
    $query->fields('u', array('uid', 'name', 'pass', 'mail', 'created', 'access', 'login', 'status', 'picture', 'init'))
      ->condition('u.status', 1, '=')
      ->condition('u.uid', 1, '>')
      ->orderBy('u.uid', 'ASC');

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
    /* avoid joins, we don't have permission to write to source */
    $this->source = new MigrateSourceSQL($query, array(), NULL, array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationUser(array('md5_passwords' => TRUE));

    $this->addFieldMapping('is_new')->defaultValue(TRUE);
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

  // massage the user roles before saving
  public function prepareRow($current_row) {
    // Get table mapping old rids to new ones
    $query = Database::getConnection('default')
      ->select('migrate_map_roles', 'r')
      ->fields('r', array('sourceid1', 'destid1'));
    $ridmap = $query->execute();

    // Get old roles vector for current user
    $source_id = $current_row->uid;
    $query = Database::getConnection('default','for_migration')
      ->select('users_roles', 'r')
      ->fields('r', array('uid', 'rid'))
      ->havingCondition('r.uid', $source_id, '=');
    $results = $query->execute();

    $roles = array();

    // add the anonymous role and authenticated roles for everyone
    // FIXME: this has no effect
    $roles['1'] = '1';
    $roles['2'] = '2';

    // Map old roles and add them
    foreach ($results as $row) {
      foreach ($ridmap as $ridmaprow) {
         if ($row->rid == $ridmaprow->sourceid1) {
            $roles[$ridmaprow->destid1] = $ridmaprow->destid1;
            break;
         }
      }
    }
    $current_row->roles = $roles;
    return TRUE;
  }
}
?>
