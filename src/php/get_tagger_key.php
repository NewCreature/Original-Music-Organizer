<?php

/********************************
** Template for use with T3Net **
********************************/

/* set up UTF-8 */
ini_set('default_charset', 'utf-8');

/* $db_host, $db_user, $db_pass, and $db_database defined in this file */
require 'settings.php';

/* database info */
$db_name = "omo_taggers"; // name of our database (database->name->fields)
$db_fields = array('name'); // fields we are interested in

/* arguments passed through the URL */
$arguments = array('limit', 'order_field', 'ascend', 'other');

/* fields we spit out */
$output_fields = array('tagger');
$output_header = "OMO Tagger Key";

/* settings */
$ascend = false;
$limit = 0;
$order_field = "tagger";

/* create key from arguments */
if(strlen($_GET['name']) == 0)
{
	die("Error: Incorrect syntax.\r\n");
}

/* Connect to database. */
$mysqli = new mysqli($db_host, $db_user, $db_pass, $db_database);
if($mysqli->connect_errno)
{
    print "Failed to connect to database: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error;
}
$mysqli->set_charset('utf8');

/* generate tagger key */
$query = "SELECT * FROM " . $db_name;
$query .= " WHERE dummy = '66'";
$result = $mysqli->query($query) or die("Error: Failed to generate tagger key.\r\n");
$tagger_key = md5($_GET['name'] . ':' . time() . '.' . mysqli_num_rows($result));

/* add new tagger to database */
$query = "INSERT INTO " . $db_name . " SET dummy = '66'";
$query .= ", `name` = '" . $mysqli->real_escape_string($_GET['name']) . "'";
$query .= ", `tagger_key` = '" . $tagger_key . "'";

$result = $mysqli->query($query) or die("Error: Failed to add.\r\n");

$output = $output_header . "\r\n\r\n";

if(mysqli_affected_rows($mysqli) > 0)
{
	$output .= "\ttagger_key: " . $tagger_key . "\r\n\r\n";
	echo $output;
}
else
{
	echo "Error: Failed to add (2).\r\n";
}

?>
