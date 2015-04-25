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
        //  drush migrate-import --group=UUSMMigrate
        // The title is a required argument which is displayed for the group in the
        // UI. You may also have additional arguments for any other data which is
        // common to all migrations in the group.
        'groups' => array(
            'UUSMMigrate' => array(
                'title' => t('UUSMMigrate')
            )
        ),
        
        'field handlers' => array(
            'DateMigrateFieldHandler',
        ),
        // Here we register the individual migrations. The keys (SermonImport_First,
        // etc.) are the machine names of the migrations, and the class_name
        // argument is required. The group_name is optional (defaulting to 'default')
        // but specifying it is a best practice.
        'migrations' => array(
           
           'Users' => array(
            'class_name' => 'UsersMigration',
                'group_name' => 'UUSMMigrate'
           ),

            'Roles' => array(
                'class_name' => 'RolesMigration',
                'group_name' => 'UUSMMigrate'
            ),
        
            'Sermon' => array(
                'class_name' => 'SermonMigration',
                'group_name' => 'UUSMMigrate'
            ),
            
            'Calendar' => array(
                'class_name' => 'CalendarMigration',
                'group_name' => 'UUSMMigrate'
            ),
        )
    );
    return $api;
}
