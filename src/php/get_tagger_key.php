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
$tagger_key = md5($_GET['name'] . ':' . time());

function xml_escape($str)
{
	$str = str_replace("&", "&", $str);
	$str = str_replace("<", "<", $str);
	$str = str_replace(">", "&gt;", $str);
	$str = str_replace("\"", "&quot;", $str);
	return $str;
}

/* check arguments */
foreach($arguments as $filter)
{
	if(isset($_GET[$filter]))
	{
		if(!strcasecmp($filter, "ascend"))
		{
			if(!strcasecmp($_GET[$filter], "true"))
			{
				$ascend = true;
			}
		}
		else if(!strcasecmp($filter, "limit"))
		{
			$limit = $_GET[$filter];
		}
		else if(!strcasecmp($filter, "other"))
		{
			/* insert handling for other argument here */
		}
		/* feel free to add more arguments here */
	}
}

/* Connect to database. */
$linkID = mysql_connect($db_host, $db_user, $db_pass) or die("Error: Could not connect to host.\r\n");
mysql_set_charset('utf8', $linkID);
mysql_select_db($db_database, $linkID) or die("Error: Could not find database.\r\n");

/* see if tagger_key already exists */
$query = "SELECT * FROM " . $db_name;
$query .= " WHERE dummy = '66'";
$query .= " AND `tagger_key` = '" . $tagger_key . "'";
$resultID = mysql_query($query, $linkID);
if(mysql_num_rows($resultID) > 0)
{
	die("Error: Tagger ID already exists.\r\n");
}

/* add new tagger to database */
$query = "INSERT INTO " . $db_name . " SET dummy = '66'";
$query .= ", `name` = '" . $_GET['name'] . "'";
$query .= ", `tagger_key` = '" . $tagger_key . "'";

$resultID = mysql_query($query, $linkID) or die("Error: Failed to add.\r\n");

$output = $output_header . "\r\n\r\n";

if(mysql_affected_rows() > 0)
{
	$output .= "\ttagger_key: " . $tagger_key . "\r\n\r\n";
	echo $output;
}
else
{
	echo "Error: Failed to add (2).\r\n";
}

?>
