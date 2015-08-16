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
function church_migrate_migrate_api()
{
require 'generated_taxonomy_migration_class_list.inc';
    $api = array(
        'api' => 2,
        // Migrations can be organized into groups. The key used here will be the
        // machine name of the group, which can be used in Drush:
        //  drush migrate-import --group=ChurchMigrate
        // The title is a required argument which is displayed for the group in the
        // UI. You may also have additional arguments for any other data which is
        // common to all migrations in the group.
        'groups' => array(
            'ChurchMigrate' => array(
                'title' => t('ChurchMigrate')
            )
        ),
        
        // Here we register the individual migrations. The keys
        // are the machine names of the migrations, and the class_name
        // argument is required. The group_name is optional (defaulting to 'default')
        // but specifying it is a best practice.
        'migrations' => array(
            'Roles' => array(
                'class_name' => 'ChurchRolesMigration',
                'group_name' => 'ChurchMigrate'
            ),
            'Users' => array(
                'class_name' => 'ChurchUsersMigration',
                'group_name' => 'ChurchMigrate'
            ),
            'Rooms' => array(
                'class_name' => 'ChurchRoomsMigration',
                'dependencies' => array('Users'),
                'description' => t('Migrate Rooms'),
                'destination_type' => 'room',
                'group_name' => 'ChurchMigrate',
                'source_connection' => 'for_migration',
                'source_type' => 'room',
                'source_version' => 6,
                'user_migration' => 'Users',
            ),
            'Menu' => array(
                'class_name' => 'DrupalMenu6Migration',   # yes, we're mixing in a migrate_d2d class!
                'description' => t('Migrate menus from Drupal 6'),
                'group_name' => 'ChurchMigrate',
                'source_connection' => 'for_migration',
                'source_version' => 6,
            ),
            'MenuLinks' => array(
                'class_name' => 'DrupalMenuLinks6Migration',   # yes, we're mixing in a migrate_d2d class!
                'dependencies' => array('Menu'),
                'description' => t('Migrate menu links from Drupal 6'),
                'group_name' => 'ChurchMigrate',
                'source_connection' => 'for_migration',
                'source_version' => 6,
            ),
            'Events' => array(
                'class_name' => 'ChurchEventsMigration',
                'dependencies' => array('MenuLinks', 'Rooms'),
                'description' => t('Migrate Events'),
                'destination_type' => 'event',
                'group_name' => 'ChurchMigrate',
                'source_connection' => 'for_migration',
                'source_type' => 'event',
                'source_version' => 6,
                'user_migration' => 'Users',
            ),
        ) + $generated_array
    );
    return $api;
}
