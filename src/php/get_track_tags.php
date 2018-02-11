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
$db_fields = array('track_id'); // fields we are interested in

/* arguments passed through the URL */
$arguments = array('limit', 'order_field', 'ascend', 'other');

/* fields we spit out */
$output_fields = array('tagger', 'album_artist', 'artist', 'album', 'disc', 'track', 'title', 'genre', 'year', 'copyright', 'comment', 'loop_start', 'loop_end', 'fade_time', 'split_track_info', 'detected_length');
$output_header = "OMO Tags";

/* settings */
$ascend = false;
$limit = 0;
$order_field = "id";

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
$mysqli = new mysqli($db_host, $db_user, $db_pass, $db_database);
if($mysqli->connect_errno)
{
    print "Failed to connect to database: (" . $mysqli->connect_errno . ") " . $mysqli->connect_error;
}
$mysqli->set_charset('utf8');

/* Build query. */
$query = "SELECT * FROM " . $db_name;

$query .= " WHERE dummy = '66'";

/* Require any $db_columns that are passed as arguments to match the arguments'
   setting. E.g. game=my_game means the game field must equal my_game to be
   selected. */
foreach($db_fields as $filter)
{
	if(isset($_GET[$filter]))
	{
		$query .= " AND `" . $filter. "` = '" . $mysqli->real_escape_string($_GET[$filter]) . "'";
	}
}

/* ascending order or descending order (some games you will want the smallest score first) */
if($ascend)
{
	$query .= " ORDER BY `" . $db_name . "`.`" . $order_field . "` ASC, `" . $db_name . "`.`id` DESC";
}
else
{
	$query .= " ORDER BY `" . $db_name . "`.`" . $order_field . "` DESC, `" . $db_name . "`.`id` DESC";
}

/* optionally impose limit on number of results returned */
if($limit > 0)
{
	$query .= " LIMIT 0 , " . $mysqli->real_escape_string($limit);
}

$result = $mysqli->query($query) or die("Error: Data not found.\r\n");

if(mysqli_num_rows($result) == 0)
{
	die("Error: No track info.\r\n");
}

$output = $output_header . "\r\n\r\n";

for($x = 0; $x < mysqli_num_rows($result); $x++)
{
    $row = mysqli_fetch_assoc($result);
	foreach($output_fields as $e)
	{
		if($row[$e])
		{
			$output .= "\t" . $e . ": ";
			if(strlen($row[$e]) > 0)
			{
				$output .= $row[$e];
			}
			$output .= "\r\n";
		}
	}
	$output .= "\r\n";
}

echo $output;

?>
