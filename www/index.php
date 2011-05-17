<?PHP
	$db_host = 'localhost';
	$db_name = 'dmoz';
	$db_user = 'dmozdb';
	$db_pass = 'piffpaff';

	$base_id = 912082;
	$base_title = "Computer";
	$base_topic = "World/Deutsch/Computer";
	$lastUpdate = "";

	$connection = mysql_connect($db_host, $db_user, $db_pass) or die("Couldn't connect to server...");
	$db = mysql_select_db($db_name) or die("Couldn't select database...");

	$levels = array('narrow', 'narrow1', 'narrow2');

	$catid = $_GET["id"];
	if ($catid == "")
		$catid=$base_id;

	$header = "";
	$searchid = $catid;
	while (1) {
		if ($searchid==$base_id) {
			$header = '<A HREF="index.php?id='.$base_id.'">'.$base_title.'</A>' . $header;
			break;
		}

		$cursor = mysql_query("SELECT parentId, title FROM dmoz_structure where topicId='$searchid'");
		if ($row = mysql_fetch_array($cursor)) {
			$header = ':<A HREF="index.php?id='.$searchid.'">'.$row["title"].'</A>'.$header;
			$searchid = $row["parentId"];
		} 
		else
			break;
	}
	
	if ($catid == $base_id) {
		$topic=$base_topic;
		$title=$base_title;
	}
	else {
		$cursor = mysql_query("SELECT title, topic, lastUpdate  FROM dmoz_structure where topicId='$catid'");
	
		if ( $row = mysql_fetch_array($cursor) ) {
			$title = $row["title"];
			$topic = $row["topic"];
			$lastUpdate = $row["lastUpdate"];
		}
		else {
			print("topic not found");
			die;
		}
	}

	print("<HTML><HEAD><TITLE>NB Film Suche - $title</TITLE>\n");
	print("\t<META name=\"keywords\" content=\"$title, film, video, kreuzberg, postproduction\">\n");
PHP?>
	<META http-equiv="content-type" content="text/html; charset=UTF-8">
	<META name="Content-Language" content="de">
	<script type="text/javascript">
	<!--
	if(top == self){
<?PHP
	//print("\twindow.location.href = '/search.php?catid=$catid';\n}");
PHP?>
	//-->
	</script>
</HEAD>
<BODY>
<form method="post" action="/cgi-bin/search.pl">
<font size="-1">
Match: <select name="method">
<option value="and">All
<option value="or">Any
<option value="boolean">Boolean
</select>
Format: <select name="format">
<option value="builtin-long">Long
<option value="builtin-short">Short
</select>
Sort by: <select name="sort">
<option value="score">Score
<option value="time">Time
<option value="title">Title
<option value="revscore">Reverse Score
<option value="revtime">Reverse Time
<option value="revtitle">Reverse Title
</select>
</font>
<input type="hidden" name="config" value="htdig">
<input type="hidden" name="restrict" value="">
<input type="hidden" name="exclude" value="">
<br>
Search:
<input type="text" size="30" name="words" value="">
<input type="submit" value="Search">
</form>
<hr noshade size="4">
<?php print("<H2>$header</H2>"); php?>
<P><HR>
<?php
	for ($level=2; $level >= 0; $level--) { 
		$cursor = mysql_query("SELECT topicId, title FROM dmoz_structure WHERE parentId='$catid' AND level='{$levels[$level]}'");
		print("<UL>\n");
		$line = 0;
		while ( $row = mysql_fetch_array($cursor) ) {
			$line = 1;
			print("<LI><a href=\"index.php?id=".$row["topicId"]."\">".$row["title"]."</a></LI>\n");
		}
		print ("</UL>\n");
		if ($line) print("<HR>\n");
	}
php?>
<P>
<UL>
<?php
        $cursor = mysql_query("SELECT title,  link, description as descr from dmoz_content where parentId='$catid' ORDER BY title");
        while ( $row = mysql_fetch_array($cursor) ) {
                print("<LI><a href=\"".$row["link"]."\">".$row["title"]."</a> ".$row["descr"]."</LI>\n");
        }
php?>
</UL>
<p align="center"><table border="0" bgcolor="#336600" cellpadding="3" cellspacing="0">
<tr>
<td>
<table width="100%" cellpadding="2" cellspacing="0" border="0">
<tr align="center">
<td><font face="sans-serif, Arial, Helvetica" size="2" color="#FFFFFF">Help build the largest human-edited directory on the web.</font></td>
</tr>
<tr bgcolor="#CCCCCC" align="center">
<td><font face="sans-serif, Arial, Helvetica" size="2">
<?PHP
	print("<a href=\"http://dmoz.org/cgi-bin/add.cgi?where=$topic\">Submit a Site</a> -");
	print("<a href=\"http://dmoz.org/about.html\"><b>Open Directory Project</b></a> -");
	print("<a href=\"http://dmoz.org/cgi-bin/apply.cgi?where=$topic\">Become an Editor</a> </font>");
PHP?>
</td></tr>
</table>
</td>
</tr>
</table>
<center><font color="#666666" size="-1">
<?PHP 
	if ($lastUpdate != "")
		print("\tLetzte &Auml;nderung: ".$lastUpdate."\n\t - ");
	print("<a href=\"http://editors.dmoz.org/editors/editcat.cgi?cat=".$topic."\">editieren</a>\n");
?>
</font></center>
</BODY>
</HTML>
<?php mysql_close($connection); php?>
