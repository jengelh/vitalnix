<?php
$section_index = array(0, 0);

function h1($title)
{
	global $section_index;
	$section_index[1] = 0;
	echo "<h1>", ++$section_index[0], " &nbsp; ",
	     htmlspecialchars($title, ENT_NOQUOTES), "</h1>";
}

function h2($title)
{
	global $section_index;
	echo "<h2>", $section_index[0], ".", ++$section_index[1], " &nbsp; ",
	     htmlspecialchars($title, ENT_NOQUOTES), "</h2>";
}
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<title>Vitalnix User Management Suite 3.1</title>
<link rel="stylesheet" href="Base-format-screen.css" type="text/css" title="Default" />
<link rel="alternate stylesheet" href="Base-format-printer.css" type="text/css" media="screen" title="Print" />
<link rel="stylesheet" href="Base-format-printer.css" type="text/css" media="print" title="Print" />
</head>

<body>

<p class="ident"><span
class="titleN">vitalni</span><span class="titleX">x</span> <span
class="titleS">user management suite</span> <span
class="titleV">3.1.0</span></p>

<hr style="display: none" />

<div id="content">
