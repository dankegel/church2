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
        
        'field handlers' => array(
            'DateMigrateFieldHandler',
        ),
        // Here we register the individual migrations. The keys (Wusel_Step1_User,
        // etc.) are the machine names of the migrations, and the class_name
        // argument is required. The group_name is optional (defaulting to 'default')
        // but specifying it is a best practice.
        'migrations' => array(
            
            'CalendarImport_First' => array(
                'class_name' => 'CalendarImport_FirstMigration',
                'group_name' => 'UUSMMigrate'
            ),
            
           'CalendarImport_Date' => array(
            
                'class_name' => 'CalendarImport_DateMigration',
                'group_name' => 'UUSMMigrate'
            ),
            
            'CalendarImport_Contact_Collection'=> array(
                 'class_name' => 'CalendarImport_Contact_CollectionMigration',
                 'group_name' => 'UUSMMigrate'
                 ),
            
            
            
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
        $this->description = t("import data to put into Event fields");
        $options = array('track_changes' => 1); //add options

        //$query->join('media', 'm', 'u.id =m.user');
        //$query->addExpression('GROUP_CONCAT(DISTINCT m.url)', 'multipleimages');
        $query = Database::getConnection('default', 'for_migration')->select('node', 'n')->fields('n', array(
            'nid',
            'uid',
            'status',
            'created',
            'changed',
            'promote',
            'sticky'
        ))->fields('nr',array('title','body','teaser'))
        ->fields('c',array('nid','field_event_contact_value'))
        ->fields('e',array('nid','field_event_contact_email_email'))
        ->fields('p',array('nid','field_event_contact_phone_value'))
        ->fields('m',array('nid','field_datetime_value','field_datetime_value2','field_datetime_rrule'));
        $query->join('node_revisions','nr',"n.nid=nr.nid");
        $query->join('content_field_event_contact','c',"n.nid=c.nid");
        $query->join('content_field_event_contact_email','e',"n.nid=e.nid");
        $query->join('content_field_event_contact_phone','p',"n.nid=p.nid");
        $query->join('content_field_datetime','m', "n.nid = m.nid");
        $query->condition('n.type','event','=');
        //$query->condition('n.nid',array('12075','12084','12087','12088','12091','12135','12214'),'IN');
        //$query->range(0, 15);
        $query->addExpression("GROUP_CONCAT(m.field_datetime_value SEPARATOR '||')", 'multipledatetime');
        $query->addExpression("GROUP_CONCAT(m.field_datetime_value2 SEPARATOR '||')", 'multipledatetime2');
        $query->addExpression("GROUP_CONCAT(m.field_datetime_rrule SEPARATOR '||')", 'multipledatetimerrule');
        $query->groupBy('m.nid');
        //$query->range(0, 5);
        $this->source = new MigrateSourceSQL($query, array(), NULL, $options);//add options
        $this->destination  = new MigrateDestinationNode('myevents');
        $this->map          = new MigrateSQLMap($this->machineName, array(
            'nid' => array(
                'type' => 'varchar',
                'length' => 6,
                'not null' => TRUE,
                'description' => 'Account ID.',
                'alias' => 'n'
            )
        ), MigrateDestinationNode::getKeySchema());
        
          $this->addFieldMapping('nid', 'n.nid'); // Connecting the D6 nid to the D7 nid
          $this->addFieldMapping('is_new')->defaultValue(TRUE)->description(t('Build the new user (0|1)'));

        // Mapped fields
          $this->addFieldMapping('title','title');
          $this->addFieldMapping('uid','uid');
          $this->addFieldMapping('status','status');
          $this->addFieldMapping('created','created');
          $this->addFieldMapping('changed','changed');
          $this->addFieldMapping('promote','promote');
          $this->addFieldMapping('sticky','sticky');
          $this->addFieldMapping('body','body');
          $this->addFieldMapping('body:format')->defaultValue('full_html');
          $this->addFieldMapping('body:summary','teaser');
          $this->addFieldMapping('field_contact', 'field_event_contact_value');
          $this->addFieldMapping('field_contact_email', 'field_event_contact_email_email');
          $this->addFieldMapping('field_contact_phone', 'field_event_contact_phone_value');
          $this->addFieldMapping('field_event_date','multipledatetime')->separator('||');
          $this->addFieldMapping('field_event_date:value2','multipledatetime2')->separator('||');
          $this->addFieldMapping('field_event_date:rrule','multipledatetimerrule')->separator('||');
          




  }
  
     public function prepareRow($current_row){
     
date(DATE_ATOM, strtotime($current_row->multipledatetime));
date(DATE_ATOM, strtotime($current_row->multipledatetime2));
/*$res = split('[||]', $current_row->multipledatetime);
for ($i=0;$i< count($res);$i++){
//drush_print($res[$i] . '||');
}*/

   //drush_print_r($current_row->multipledatetime);
  
             
              
   }
  

    }
    
    

    
   
    class CalendarImport_DateMigration extends Calendar_Basic_Migration
 {
    public function __construct($arguments)
    {
        parent::__construct($arguments);
        $this->description = t("import data to put into Date fields");
        $options = array('track_changes' => 1); //add options
        $query = Database::getConnection('default', 'for_migration')->select('content_field_datetime', 'm')->fields('m',array('nid','field_datetime_value','delta','field_datetime_value2','field_datetime_rrule'));
        $query->condition('m.nid',array('12075','12084','12087','12088','12091'),'IN');
        $query->addExpression('GROUP_CONCAT(m.field_datetime_value)', 'multipledatetime');
        $query->addExpression('GROUP_CONCAT(m.field_datetime_value2)', 'multipledatetime2');
        $query->addExpression('GROUP_CONCAT(m.field_datetime_rrule)', 'multipledatetimerrule');
        $query->groupBy('m.nid');
        $this->source = new MigrateSourceSQL($query, array(), NULL, $options);//add options
        $this->softDependencies = array('CalendarImport_First'); 
          $this->destination  = new MigrateDestinationNode('myevents');
        $this->map          = new MigrateSQLMap($this->machineName, array(
            'nid' => array(
                'type' => 'varchar',
                'length' => 6,
                'not null' => TRUE,
                'description' => 'Account ID.',
                'alias' => 'n'
            )
        ), MigrateDestinationNode::getKeySchema());
       
 $this->addFieldMapping('field_event_date_value','multipledatetime')->separator(',');
 $this->addFieldMapping('field_event_date_value2','multipledatetime2')->separator(',');
 $this->addFieldMapping('field_event_date_rrule','multipledatetimerrule')->separator(',');
        
  }
  
   public function prepareRow($current_row){
   
   drush_print_r($current_row);
  
             
              
   }
 }
  
 class CalendarImport_Contact_CollectionMigration extends Calendar_Basic_Migration
{
    public function __construct($arguments)
    {
        parent::__construct($arguments);
        $this->description = t("import data to put into Event fields");
        $options = array('track_changes' => 1); //add options

        $query = Database::getConnection('default', 'for_migration')->select('node', 'n')
        ->fields('n', array('nid'))
        ->fields('c',array('field_event_contact_value'))
        ->fields('e',array('field_event_contact_email_email'))
        ->fields('p',array('field_event_contact_phone_value'));
        $query->join('content_field_event_contact','c',"n.nid=c.nid");
        $query->join('content_field_event_contact_email','e',"n.nid=e.nid");
        $query->join('content_field_event_contact_phone','p',"n.nid=p.nid");
        $query->condition('n.nid',array('12075','12084','12087','12088','12091'),'IN');
       // $query->range(0, 5);
        $this->source = new MigrateSourceSQL($query, array(), NULL, $options);//add options
        $this->softDependencies = array('CalendarImport_First'); // Declare migration 'CalendarImport_First' a dependency in migration 'Commxdb_Vehicle_Collection' to have them run in the right order, if needed:
        $this->destination  = new MigrateDestinationFieldCollection(
      'field_event_contact',
      array('host_entity_type' => 'node')
    );
        $this->map          = new MigrateSQLMap($this->machineName, array(
            'nid' => array(
                'type' => 'varchar',
                'length' => 6,
                'not null' => TRUE,
                'description' => 'Account ID.',
                'alias' => 'n'
            )
        ), MigrateDestinationFieldCollection::getKeySchema());
        
         $this->addFieldMapping('host_entity_id', 'nid')->sourceMigration('CalendarImport_Contact_Collection');
        $this->addFieldMapping('field_event_contact', 'field_event_contact_value');
        $this->addFieldMapping('field_event_contact_email', 'field_event_contact_email_email');
        $this->addFieldMapping('field_event_contact_phone', 'field_event_contact_phone_value');

        
    }
    
    
    
 
    }