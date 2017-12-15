<?php

/********************************
** Template for use with T3Net **
********************************/

/* set up UTF-8 */
ini_set('default_charset', 'utf-8');

/* database info */
$db_host = "97.74.218.213"; // address of host
$db_user = "oldcreature"; // login credentials
$db_pass = "se4theBible#66";
$db_database = "oldcreature"; // root database
$db_name = "omo"; // name of our database (database->name->fields)
$db_fields = array('track_id', 'tagger', 'album_artist', 'artist', 'album', 'disc', 'track', 'title', 'genre', 'year', 'copyright', 'comment', 'loop_start', 'loop_end', 'fade_time'); // fields we are interested in

/* command-specific data */
$update = false;

/* connect to database */
$linkID = mysql_connect($db_host, $db_user, $db_pass) or die("Error: Could not connect to host.\r\n");
mysql_set_charset('utf8', $linkID);
mysql_select_db($db_database, $linkID) or die("Error: Could not find database.\r\n");

/* Check for existing entry. */
$query = "SELECT * FROM " . $db_name;
$query .= " WHERE dummy = '66'";
$query .= " AND `track_id` = '" . mysql_real_escape_string($_GET['track_id']) . "'";
$query .= " AND `tagger` = '" . mysql_real_escape_string($_GET['tagger']) . "'";
$result = mysql_query($query, $linkID);
if(mysql_affected_rows() > 0)
{
	$update = true;
	$query = "UPDATE " . $db_name . " SET dummy = '66'";
}
else
{
	$query = "INSERT INTO " . $db_name . " SET dummy = '66'";
}

/* build query from passed fields */
foreach($db_fields as $field)
{
	if(strlen($_GET[$field]) > 0)
	{
		$query .= ", `" . mysql_real_escape_string($field) . "` = '" . mysql_real_escape_string($_GET[$field]) . "'";
	}
}

/* command-specific query options */
if($update)
{
	$query .= " WHERE track_id='" . $_GET['track_id'] . "' AND tagger='" . $_GET['tagger'] . "'";
}
print $query;

/* run the query */
$result = mysql_query($query, $linkID) or die("Error: Invalid Entry.\r\n");
if(mysql_affected_rows() == 0)
{
	// this most likely won't happen
	print "Error: Invalid Request\r\n";
	exit;
}

/* success */
print "ack:\r\n";

?>
