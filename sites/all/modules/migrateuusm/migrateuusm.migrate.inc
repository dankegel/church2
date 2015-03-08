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
        
            'SermonImport_First' => array(
                'class_name' => 'SermonImport_FirstMigration',
                'group_name' => 'UUSMMigrate'
            ),
            
            'CalendarImport_First' => array(
                'class_name' => 'CalendarImport_FirstMigration',
                'group_name' => 'UUSMMigrate'
            ),
            
         /*  'CalendarImport_Date' => array(
            
                'class_name' => 'CalendarImport_DateMigration',
                'group_name' => 'UUSMMigrate'
            ),
            
            'CalendarImport_Contact_Collection'=> array(
                 'class_name' => 'CalendarImport_Contact_CollectionMigration',
                 'group_name' => 'UUSMMigrate'
            ),*/
            
            
            
        )
    );
    return $api;
}

/**
 * Migration classes for migrating calendar events and dates and sermons
 *
 * 
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


//This imports sermons, all fields, including audio.  Audio is handled along with
//all the other fields conained in the Sermon migration. The special thing about files is
//that you have to populate the file_managed table as well as the field table (field_data_field_audio in 
//this case).  More comments below on how this is done.

class SermonImport_FirstMigration extends Calendar_Basic_Migration
{
	public function __construct($arguments)
	{
		parent::__construct($arguments);
	$this->description = t("import sermons");
	$options = array('track_changes' => 1);

//Make a query that gets everything. Do a lot of joins.  This query restricts
//to records known to have attached audio files.		
	$query = Database::getConnection('default','for_migration')->select('node', 'n')->fields('n',array(
		'nid',
		'uid',
		'status',
		'created',
		'changed',
		'promote',
		'sticky'
	     ))->fields('nr',array('title','body','teaser'))
	     ->fields('s',array('nid','field_minister_value','field_pulpit_host_value', 'field_sermon_date_value','field_sermon_theme_value'))
	     ->fields('f',array('filename','filepath','fid'))
	     ->fields('a',array('field_audio_fid'))
	     ->fields('st',array('field_sermon_text_value','nid'));
	     $query->join('node_revisions','nr','n.nid=nr.nid');
	     $query->join('content_type_sermon','s','n.nid=s.nid');
	     $query->join('content_field_audio','a','s.nid=a.nid');
	     $query->join('files','f','a.field_audio_fid=f.fid');
	     $query->join('content_field_sermon_text','st','st.nid=n.nid');
	     $query->condition('n.type','sermon','=');
	     $query->condition('st.field_sermon_text_value','','!=');
	   //  $query->condition('f.fid','475','<');
        //     $query->condition('f.fid','425','>');
	     $query->range(0, 150);
	     $this->source = new MigrateSourceSQL($query, array(), NULL, $options);//add options
	     $this->destination  = new MigrateDestinationNode('sermons');
	    
//This part creates the lookup table that keeps track of the old nid and the new nid.  By keeping track of
//this, it is possible to avoid re-importing the same record more than once, and it allows you to
//roll back the entire migration. First it maps, and then it calls on getKeySchema which consults
//Drupal structure for the Sermon content type.  I think.

	     $this->map = new MigrateSQLMap($this->machineName, array(
             'nid' => array(
                'type' => 'varchar',
                'length' => 6,
                'not null' => TRUE,
                'description' => 'Account ID.',
                'alias' => 'n'
            )
        ), MigrateDestinationNode::getKeySchema());

//Most of these mappings are self evident.  Except for field_audio

         $this->addFieldMapping('nid', 'n.nid');
         $this->addFieldMapping('is_new')->defaultValue(TRUE)->description(t('Build the new sermon (0|1)')); 
         $this->addFieldMapping('title','title');
         $this->addFieldMapping('status','status');
         $this->addFieldMapping('created','created');
         $this->addFieldMapping('changed','changed');
         $this->addFieldMapping('promote','promote');
         $this->addFieldMapping('sticky','sticky');
         $this->addFieldMapping('body','field_description');
         $this->addFieldMapping('body:format')->defaultValue('full_html');
        // $this->addFieldMapping('body:summary','teaser');
         $this->addFieldMapping('field_minister_speaker', 'field_minister_value');
         $this->addFieldMapping('field_pulpit_host', 'field_pulpit_host_value');
         $this->addFieldMapping('field_sermon_date', 'field_sermon_date_value');
         $this->addFieldMapping('field_sermon_text', 'field_sermon_text_value');
	 $this->addFieldMapping('field_sermon_text:format')->defaultValue('full_html');
         $this->addFieldMapping('field_sermon_theme', 'field_sermon_theme_value');
//field_audio maps to an asset depending on the file_class assigned to handle all the logic and data
//that has to fill up the files_managed table and the field_audio.  When you use the class MigrateFileUri
//that means the value of what's mapped to field_audio is a url, relative to the field_audio
//source_directory.  So since the source directory path is the entire url including http - because
//it's remote -- all you need to map to field_audio is the filename.

         $this->addFieldMapping('field_audio', 'filename');
         $this->addFieldMapping('field_audio:file_class')
         ->defaultValue('MigrateFileUri');
	 $this->addFieldMapping('field_audio:source_dir')->defaultValue('http://www.peggyo.net/fuller/sites/default/files/audio/');
//This is supposed to make it so you don't delete the files if you roll back but it doesn't work.
//Need to find out why this does not work.  If you roll back, all 500 mb of audio files you patiently
//imported will be erased.
	 $this->addFieldMapping('field_audio:preserve_files', TRUE);
	  
 }
    }
    
  //*****Roles Migration***referenced in UsersMigration****/
  
  //*****END Roles Migration********/
    
    //******USERS*****/
    
    class UsersMigration extends Calendar_Basic_Migration {
	public function __construct($arguments)
	{
		parent::__construct($arguments);
	$this->description = t("import Users");
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
   
    //By default, Migrate will try to directly join the source query to the map table, 
    //which is (by default) in the Drupal database, as a performance optimization. 
    //The following line disables this behavior:
    //$this->source = new MigrateSourceSQL($query, $source_fields,NULL, array('map_joinable' => FALSE));
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
       MigrateDestinationNode::getKeySchema(),
       'for_migration'
    );
    
     $this->source = new MigrateSourceSQL($query, $source_fields);
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
 //   drush_print($source_id);
    $query = Database::getConnection('default','for_migration')
      ->select('users_roles', 'r')
      ->fields('r', array('uid', 'rid'))
      ->havingCondition('r.uid', $source_id, '=');
    $results = $query->execute();
    // add the anonymous role for everyone
    $roles = array('1' => '1','2'=>'2');
    //$roles = array('2' => '2');
    foreach ($results as $row) {
      // adjust the old to new role id's
      // Webmaster (rid 8) needs role id 3 to match admin because
      //the current administrator role is 3.
      if ($row->rid == 3) {
        $roles['6'] = '6';
      }
      elseif ($row->rid == 4) {
        // group editor from 10 to 4 to match editor role in D7 installation
        $roles['8'] = '8';
      }
      elseif ($row->rid == 5) {
        // church member from 3 to 8
        $roles['9'] = '9';
      }
      elseif ($row->rid == 6) {
        // group leader 4 to 10
        $roles['12'] = '12';
      }
      
            elseif ($row->rid == 7) {
        // group leader 4 to 10
        $roles['11'] = '11';
      }
      
            elseif ($row->rid == 8) {
        // group leader 4 to 10
        $roles['13'] = '13';
      }
      
            elseif ($row->rid == 9) {
        // group leader 4 to 10
        $roles['5'] = '5';
      }
      
            elseif ($row->rid == 10) {
        // group leader 4 to 10
        $roles['7'] = '7';
      }
      
            elseif ($row->rid == 11) {
        // group leader 4 to 10
        $roles['10'] = '10';
      }
     
      else {
        // The rest are the same, mostly because there are 7 more roles from D6 that go into D7
        $roles[$row->rid] = $row->rid;
      }
    }
    $current_row->roles = $roles;
    drush_print_r($roles);
    return TRUE;
  }
} // close the ourUsersMigration class
    
    /*******END USERS****/

//The following class imports all the events and dates.  Everything gets put
//into the myevents content type.  The fancy trick here is to fetch a comma delimited list
//in the query, using just mysql. GROUP_CONCAT allows you make comma delimited lists
//out of dates you get from content_field_datetime which allows for multiple dates associated
//with the same node id.  Then you group by node id, and send the comma delimited list to the date
//field in D7.  The ability to translate a comma delimited list into records is built in.

class CalendarImport_FirstMigration extends Calendar_Basic_Migration
{
    public function __construct($arguments)
    {
        parent::__construct($arguments);
        $this->description = t("import data to put into Event fields");
        $options = array('track_changes' => 1); //add options

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
        $query->addExpression("GROUP_CONCAT(m.field_datetime_value SEPARATOR '||')", 'multipledatetime');
        $query->addExpression("GROUP_CONCAT(m.field_datetime_value2 SEPARATOR '||')", 'multipledatetime2');
        $query->addExpression("GROUP_CONCAT(m.field_datetime_rrule SEPARATOR '||')", 'multipledatetimerrule');
        $query->groupBy('m.nid');
        //$query->range(0, 5);for testing.
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

     }

}
