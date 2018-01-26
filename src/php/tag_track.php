<?php

/********************************
** Template for use with T3Net **
********************************/

/* set up UTF-8 */
ini_set('default_charset', 'utf-8');

/* $db_host, $db_user, $db_pass, and $db_database defined in this file */
require 'settings.php';

/* database info */
$db_name = "omo"; // name of our database (database->name->fields)
$db_fields = array('track_id', 'tagger', 'album_artist', 'artist', 'album', 'disc', 'track', 'title', 'genre', 'year', 'copyright', 'comment', 'loop_start', 'loop_end', 'fade_time', 'split_track_info', 'detected_length'); // fields we are interested in
$tagger_db_name = "omo_taggers";

/* require tagger ID */
if(strlen($_GET['tagger']) == 0)
{
	die("Error: No tagger ID.\r\n");
}

/* require track ID */
if(strlen($_GET['track_id']) == 0)
{
	die("Error: No track ID.\r\n");
}

/* command-specific data */
$update = false;

/* connect to database */
$mysqli = new mysqli($db_host, $db_user, $db_pass, $db_database);
if($mysqli->connect_errno)
{
    print "Failed to connect to database: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error;
}

/* validate tagger ID */
$query = "SELECT * FROM " . $tagger_db_name;
$query .= " WHERE dummy = '66'";
$query .= " AND `tagger_key` = '" . $mysqli->real_escape_string($_GET['tagger']) . "'";
$result = $mysqli->query($query);
if(mysqli_num_rows($result) == 0)
{
	die("Error: Invalid tagger ID.\r\n");
}

/* Check for existing entry. */
$query = "SELECT * FROM " . $db_name;
$query .= " WHERE `dummy` = '66'";
$query .= " AND `track_id` = '" . $mysqli->real_escape_string($_GET['track_id']) . "'";
$query .= " AND `tagger` = '" . $mysqli->real_escape_string($_GET['tagger']) . "'";
$result = $mysqli->query($query);
if(mysqli_num_rows($result) > 0)
{
	$update = true;
	$query = "UPDATE " . $db_name . " SET `dummy` = '66'";
}
else
{
	$query = "INSERT INTO " . $db_name . " SET `dummy` = '66'";
}

/* build query from passed fields */
$tag_count = 0;
foreach($db_fields as $field)
{
	if(strlen($_GET[$field]) > 0)
	{
		$query .= ", `" . $mysqli->real_escape_string($field) . "` = '" . $mysqli->real_escape_string($_GET[$field]) . "'";
		$tag_count++;
	}
	else
	{
		$query .= ", `" . $mysqli->real_escape_string($field) . "` = " . "NULL";
	}
}

/* command-specific query options */
if($tag_count <= 2)
{
	$query = "DELETE FROM " . $db_name;
}
if($update)
{
	$query .= " WHERE `track_id` = '" . $_GET['track_id'] . "' AND `tagger` = '" . $_GET['tagger'] . "'";
}
if(!$update && $tag_count <= 2)
{
	print "Error: Can't delete non-existent entry.\r\n";
	exit;
}

/* run the query */
$result = $mysqli->query($query) or die("Error: Invalid Entry.\r\n");
if(mysqli_affected_rows($mysqli) == 0)
{
	// this most likely won't happen
	print "ack: Nothing changed.\r\n";
	exit;
}

/* success */
print "ack:\r\n";

?>
