<?php
/**
 * @file
 * Our own hook implementation.
 */
/**
 * Implements hook_migrate_api()
 *
 * Returns 'api' => 2 for the 7.x-2.x branch of Migrate.
 * Registers  the migration classes for the 7.x-2.6 branch of Migrate (including
 * 7.x-2.6+xx-dev).
 */
function migrateuusm_migrate_api()
{
    $api = array(
        'api' => 2,
        // Migrations can be organized into groups. The key used here will be the
        // machine name of the group, which can be used in Drush:
        //  drush migrate-import --group=CommxMigrate
        // The title is a required argument which is displayed for the group in the
        // UI. You may also have additional arguments for any other data which is
        // common to all migrations in the group.
        'groups' => array(
            'UUSMMigrate' => array(
                'title' => t('UUSMMigrate')
            )
        ),
        // Here we register the individual migrations. The keys (Wusel_Step1_User,
        // etc.) are the machine names of the migrations, and the class_name
        // argument is required. The group_name is optional (defaulting to 'default')
        // but specifying it is a best practice.
        'migrations' => array(
            
            'CalendarImport_First' => array(
                'class_name' => 'CalendarImport_FirstMigration',
                'group_name' => 'UUSMMigrate'
            )
            
            
            
        )
    );
    return $api;
}

/**
 * Migration classes for migrating users and profiles
 *
 * based on: drupal.org/node/1269066#comment-4988994
 * and:      drupal.org/node/1190958#comment-4616032
 */
/**
 * Abstract class as a base for all our migration classes
 */

abstract class Calendar_Basic_Migration extends Migration
{
    public function __construct($arguments)
    {
        // Always call the parent constructor first for basic setup
        parent::__construct($arguments);
        // Avoid known line ending issue: "Invalid data value" at drupal.org/node/1152158#InvalidDataValue
        ini_set('auto_detect_line_endings', TRUE);
    }
}

class CalendarImport_FirstMigration extends Calendar_Basic_Migration
{
    public function __construct($arguments)
    {
        parent::__construct($arguments);
        $this->description = t("import data to put into Date fields");
        $options = array('track_changes' => 1); //add options

        //$query->join('media', 'm', 'u.id =m.user');
        //$query->addExpression('GROUP_CONCAT(DISTINCT m.url)', 'multipleimages');
        $query = Database::getConnection('default', 'for_migration')->select('node', 'n')->fields('n', array(
            'nid',
            'title'
        ));
        $this->source = new MigrateSourceSQL($query, array(), NULL, $options);//add options
        $this->destination  = new MigrateDestinationNode('article');
        $this->map          = new MigrateSQLMap($this->machineName, array(
            'nid' => array(
                'type' => 'varchar',
                'length' => 6,
                'not null' => TRUE,
                'description' => 'Account ID.',
                'alias' => 'u'
            )
        ), MigrateDestinationNode::getKeySchema());

  }}