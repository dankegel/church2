<?php
/* Migrate roles.  Maybe roles don't need migration; they change so seldom
 * they could have been hardcoded.  But I wrote it when I was just learning
 * about migration and was looking at trees rather than the forest.
 *
 */
/* This may plotz if there are already roles,
 * see https://www.drupal.org/node/1825698
 * so please ignore errors migrating the first two users (anonymous and authenticated user)
 */
class ChurchRolesMigration extends Migration {
  public function __construct($arguments)
  {
    parent::__construct($arguments);
    $this->description = t("import roles");
    $options = array('track_changes' => 1);
    $query = Database::getConnection('default', 'for_migration')->select('role', 'r');
    $query->fields('r', array('rid', 'name'));
    // Do lowest ID first so they end up having same IDs.
    $query->orderBy('r.rid', 'ASC');
    /* avoid joins, we don't have permission to write to source */
    $this->source = new MigrateSourceSQL($query, array(), NULL, array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationRole();
    $this->map = new MigrateSQLMap($this->machineName,
      array('rid' => array(
        'type' => 'int',
        'not null' => TRUE,
        'description' => 'Role ID'
      )),
      MigrateDestinationRole::getKeySchema()
    );
    $this->addFieldMapping('name', 'name');
  }
  /* fixme: would want to assign a weight in prepareRow? */


  // map primary key (oh my!)
  public function prepareKey($source_key, $row) {
    // Prefix if needed to avoid stomping on new system's predefined roles
    if ($row->rid < 3) $row->name = "old " . $row->name;
    // Build the key array and return it
    $key = array();
    $key['rid'] = $row->rid;
    return $key;
  }
}
?>
