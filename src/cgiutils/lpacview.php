<?php
/*
 *	lpacview.php - Web interface for print accounting statistics
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */

function force_https()
{
	if (!isset($_SERVER["HTTPS"]) || $_SERVER["HTTPS"] != "on") {
		header("HTTP/1.1 426 Upgrade Required");
		header("Location: https://".$_SERVER["HTTP_HOST"].$_SERVER["REQUEST_URI"]);
		exit();
	}
}

/*
 * config_read - read config file and return data
 * @file:       file to read
 *
 * Returns %false on error, or an associative array with the key=value
 * pairs on success.
 */
function config_read($file)
{
	$ret = array();
	$fh  = fopen($file, "r");

	if ($fh === false)
		return false;

	while (($line = fgets($fh)) !== false) {
		$line = trim($line);
		if (strlen($line) == 0 || substr($line, 0, 1) == "#")
			continue;
		list($key, $value) = explode("=", $line, 2);

		/* Warning: Cheap dequoting */
		$value = trim($value);
		if(substr($value, 0, 1) == "\"")
			$value = stripcslashes(substr($value, 1, -1));

		$ret[strtolower(chop($key))] = $value;
	}

	fclose($fh);
	return $ret;
}

function login_form()
{
	?>
	<form method="POST">
	<input type="hidden" name="auth" value="1" />

	<h1>Vitalnix Print Accounting Overview</h1>

	<table>
		<tr>
			<td align="right">Username:</td>
			<td><input type="text" name="username" /></td>
		</tr>
		<tr>
			<td align="right">Password:</td>
			<td><input type="password" name="password" /></td>
		</tr>
		<!-- does not work, argh
		<tr>
			<td align="right"><input type="checkbox"
				name="persistent" value="1" /></td>
			<td>Persistent cookie</td>
		</tr>
		-->
		<tr>
			<td>&nbsp;</td>
			<td><input type="submit" value="Authenticate" /></td>
		</tr>
	</form>
	<?php
}

function do_authenticate($user, $password)
{
	$fh = popen("/usr/src/vitalnix/final/tryauth", "w");
	if ($fh === false)
		return false;

	fprintf($fh, "$user\n$password\n");
	$ret = pclose($fh);
	return $ret == 0;
}

function do_logout($cont = false)
{
	session_start();
	session_destroy();
	session_unset();
	if ($cont)
		return;

	header("HTTP/1.1 302 Found");
	if (($ret = strpos($_SERVER["REQUEST_URI"], '?')) !== false)
		header("Location: ".substr($_SERVER["REQUEST_URI"], 0, $ret));
	else
		header("Location: ".$_SERVER["REQUEST_URI"]);
	exit();
}

function do_authenticate_login()
{
	do_logout(true);
	if (!do_authenticate($_POST["username"], $_POST["password"]))
		return;
	if (isset($_POST["persistent"])) {
		session_set_cookie_params(86400 * 7);
		session_start();
		$_SESSION["persistent"] = true;
	} else {
		session_start();
	}
	$_SESSION["user"] = $_POST["username"];

	header("HTTP/1.1 302 Found");
	header("Location: ".$_SERVER["REQUEST_URI"]);
	exit();
}

function is_root()
{
	if (!isset($_SESSION["user"]))
		return false;
	if ($_SESSION["user"] == "root" || $_SESSION["user"] == "sysop")
		return true;
	return false;
}

function is_verbose()
{
	return isset($_GET["v"]);
}

function sqlencode($val)
{
	return mysql_real_escape_string($val);
}

function helptext()
{
	?>
	<p>
	<?php if (is_verbose()) { ?>
		CMYK: color parts;
	<?php } ?>
		S: output complete;
		<span style="color: green;">✓</span> = yes;
		<span style="color: red;">✘</span> = no;<br />
	<i>Ink</i> is measured in fully-tinted ISO A4 pages.</p>
	<?php
}

function time_period($user = "")
{
	global $DBLINK;

	if ($user != "")
		$user = sprintf("where user='%s'", sqlencode($user));

	/* Get start of period */
	$ret = mysql_query("select time from printlog $user ".
	       "order by time limit 1", $DBLINK);
	if ($ret === false)
		die(mysql_error());
	$data = mysql_fetch_array($ret);
	if ($data === false)
		return array(NULL, NULL);
	$time_start = $data["time"];

	/* Get end of period */
	$ret = mysql_query("select time from printlog $user ".
	       "order by time desc limit 1", $DBLINK);
	if ($ret === false)
		die(mysql_error());
	$data = mysql_fetch_array($ret);
	if ($data === false)
		die("Unexpected SQL result");
	$time_end = $data["time"];

	return array($time_start, $time_end);
}

function order_command($c)
{
	if ($c == "pa")
		return "order by pages";
	if ($c == "pd")
		return "order by pages desc";
	if ($c == "da")
		return "order by time";
	if ($c == "dd")
		return "order by time desc";
	if ($c == "ua")
		return "order by user";
	if ($c == "ud")
		return "order by user desc";
	if ($c == "ia")
		return "order by ink";
	if ($c == "id")
		return "order by ink desc";
	return "order by ink desc";
}

function root_query()
{
	global $DBLINK;

	if (!isset($_GET["sort"]))
		$_GET["sort"] = "id";
	$order = order_command($_GET["sort"]);
	$query = "select user, sum(total) as ink, sum(pages) as pages from ".
	         "(select * from printlog group by user, jid, title) as ".
	         "plog group by user $order";
	$ret   = mysql_query($query, $DBLINK);
	if ($ret === false)
		echo "<p><b>SELECT query failed:</b><br />",
		     htmlspecialchars($query, ENT_NOQUOTES), "<br />",
		     "<b>", htmlspecialchars(mysql_error(), ENT_NOQUOTES),
		     "</b></p>";
	return $ret;
}

function root_view()
{
	list($time_start, $time_end) = time_period();
	$ret  = root_query();
	$isum = 0;
	$psum = 0;

	?>
	<p><a href="?logout">Logout</a></p>

	<p>Time period: <b><?= $time_start ?></b>&nbsp;–
	<b><?= $time_end ?></b></p>

	<form method="POST">
	<table border="1" class="with_border">
		<tr>
			<th valign="bottom">Delete</th>
			<th nowrap="nowrap" valign="bottom">Username
				<a href="?sort=ua">▲</a><a
				href="?sort=ud">▼</a></th>
			<th valign="bottom">Full name</th>
			<th nowrap="nowrap" valign="bottom">Ink
				<a href="?sort=ia">▲</a><a
				href="?sort=id">▼</a></th>
			<th nowrap="nowrap" valign="bottom">Pages
				<a href="?sort=pa">▲</a><a
				href="?sort=pd">▼</a></th>
		</tr>
		<?php
		while (($data = mysql_fetch_array($ret)) !== false) {
			$isum += $data["ink"];
			$psum += $data["pages"];
			$pent  = posix_getpwnam($data["user"]);
			$gent  = posix_getgrgid($pent["gid"]);
		?>
		<tr>
			<td><input type="checkbox" name="d_user[]" value="<?= $data['user'] ?>"></td>
			<td><a href="?user=<?= $data['user'] ?>"><?= $data["user"] ?></a> (<?= $gent["name"] ?>)</td>
			<td><?= htmlspecialchars($pent["gecos"], ENT_NOQUOTES) ?></td>
			<td align="right"><?= sprintf("%.2f", $data["ink"]) ?></td>
			<td align="right"><?= $data["pages"] ?></td>
		</tr>
		<?php } ?>
		<tr>
			<td><input type="checkbox" name="d_trunc" value="1" /></td>
			<td colspan="2"><b>All users</b></td>
			<td align="right"><b><?= sprintf("%.2f", $isum) ?></b></td>
			<td align="right"><b><?= $psum ?></b></td>
		</tr>
		<tr>
			<td>&nbsp;</td>
			<td colspan="4"><input type="submit"
				value="Delete selected entries" /></td>
		</tr>
	</table>
	</form>

	<p>Note: Values listed in this view include unsuccessful jobs.</p>

	<?php
	helptext();
}

function user_query($user)
{
	global $DBLINK;

	if (!isset($_GET["sort"]))
		$_GET["sort"] = "dd";
	$order = order_command($_GET["sort"]);
	$user  = sprintf("where user='%s'", sqlencode($user));
	$query = "select time, queue, jid, user, title, total as ink, ".
	         "cyan, magenta, yellow, black, pages, confirmed ".
	         "from printlog $user $order";
	$ret   = mysql_query($query, $DBLINK);
	if ($ret === false)
		echo "<p><b>SELECT query failed:</b><br />",
		     htmlspecialchars($query, ENT_NOQUOTES), "<br />",
		     "<b>", htmlspecialchars(mysql_error(), ENT_NOQUOTES),
		     "</b></p>";
	return $ret;
}

function user_query2($user, $confirmed = false)
{
	global $DBLINK;

	$select = "";
	if ($confirmed !== false && $confirmed == 0)
		$select = "and confirmed=0";
	else if ($confirmed !== false && $confirmed == 1)
		$select = "and confirmed=1";

	$user  = sprintf("where user='%s'", sqlencode($user));
	$query = "select count(*) as jobs, sum(cyan) as cyan, sum(magenta) ".
	         "as magenta, sum(yellow) as yellow, sum(black) as black, ".
	         "sum(total) as total, sum(pages) as pages from printlog ".
	         "$user $select";
	$ret   = mysql_query($query, $DBLINK);
	if ($ret === false)
		echo "<p><b>SELECT query failed:</b><br />",
		     htmlspecialchars(mysql_error(), ENT_NOQUOTES), "<br />",
		     "<b>", htmlspecialchars($query, ENT_NOQUOTES), "</b></p>";
	return $ret;
}

/*
 * strip_smbprn - Remove smbprn.00000000 prefix
 */
function strip_smbprn($s)
{
	return preg_replace('/^smbprn\.\d+/', "", $s);
}

function user_view($user)
{
	list($time_start, $time_end) = time_period($user);
	$job_list = user_query($user);
	$cum_succ = user_query2($user, 1);
	$cum_fail = user_query2($user, 0);
	$cum_all  = user_query2($user, false);
	?>

	<p><a href="?logout">Logout</a></p>

	<?php if (is_root()) { ?>
	<p><a href="?">User overview</a></p>
	<?php } ?>

	<p>Time period: <b><?= $time_start ?></b>&nbsp;–
	<b><?= $time_end ?></b>&nbsp;—
	<?php if (is_root()) { ?>
		<?php if (is_verbose()) { ?>
			<a href="?user=<?= $_GET['user'] ?>">Normal view</a>
		<?php } else { ?>
			<a href="?user=<?= $_GET['user'] ?>&amp;v=1">Verbose view</a>
		<?php } ?>
	<?php } else { ?>
		<?php if (is_verbose()) { ?>
			<a href="?">Normal view</a>
		<?php } else { ?>
			<a href="?v=1">Verbose view</a>
		<?php } ?>
	<?php } ?>
	</p>

	<form method="POST">
	<table border="1" class="with_border">
		<tr>
			<?php if (is_root()) { ?>
				<th valign="bottom">Delete</th>
			<?php } ?>
			<th nowrap="nowrap" valign="bottom">Date
				<a href="?user=<?= $user ?>&amp;sort=da">▲</a><a
				href="?user=<?= $user ?>&amp;sort=dd">▼</a></th>
			<th valign="bottom">Title</th>
			<th nowrap="nowrap" valign="bottom">Pages
				<a href="?user=<?= $user ?>&amp;sort=pa">▲</a><a
				href="?user=<?= $user ?>&amp;sort=pd">▼</a></th>
			<th nowrap="nowrap" valign="bottom">Ink
				<a href="?user=<?= $user ?>&amp;sort=ia">▲</a><a
				href="?user=<?= $user ?>&amp;sort=id">▼</a></th>
			<?php if (is_verbose()) { ?>
				<th valign="bottom">C</th>
				<th valign="bottom">M</th>
				<th valign="bottom">Y</th>
				<th valign="bottom">K</th>
				<th valign="bottom">Job ID</th>
			<?php } ?>
			<th valign="bottom">S</th>
		</tr>
		<?php
		while (($data = mysql_fetch_array($job_list)) !== false) {
		?>
		<tr>
			<?php if (is_root()) { ?>
				<td><input type="checkbox" name="d_job[]" value="<?= $data['queue'] ?>/<?= $data['jid'] ?>/<?= $data['user'] ?>"></td>
			<?php } ?>
			<td><?= htmlspecialchars($data["time"], ENT_NOQUOTES) ?></td>
			<td><?= htmlspecialchars(strip_smbprn($data["title"]), ENT_NOQUOTES) ?></td>
			<td align="right"><?= $data["pages"] ?></td>
			<td align="right"><?= sprintf("%.3f", $data["total"]) ?></td>
			<?php if (is_verbose()) { ?>
				<td align="right"><?= sprintf("%.3f", $data["cyan"]) ?></td>
				<td align="right"><?= sprintf("%.3f", $data["magenta"]) ?></td>
				<td align="right"><?= sprintf("%.3f", $data["yellow"]) ?></td>
				<td align="right"><?= sprintf("%.3f", $data["black"]) ?></td>
				<td><?= htmlspecialchars($data["queue"]."/".$data["jid"], ENT_NOQUOTES) ?></td>
			<?php } ?>
			<td align="center"><?= $data["confirmed"] ? "<span style=\"color: green;\">✓</span>" : "<span style=\"color: red;\">✘</span>" ?></td>
		</tr>
		<?php
		}
		if (($data = mysql_fetch_array($cum_succ)) !== false) {
		?>
		<tr>
			<?php if (is_root()) { ?>
				<td><input type="checkbox" name="d_usersucc[]" value="<?= $_GET['user'] ?>" /></td>
			<?php } ?>
			<td colspan="2"><b>All successful jobs</b> (<?= $data["jobs"] ?>)</td>
			<td align="right"><?= $data["pages"] ?></td>
			<td align="right"><?= sprintf("%.2f", $data["total"]) ?></td>
			<?php if (is_verbose()) { ?>
				<td align="right"><?= sprintf("%.2f", $data["cyan"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["magenta"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["yellow"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["black"]) ?></td>
				<td>&nbsp;</td>
			<?php } ?>
			<td>&nbsp;</td>
		</tr>
		<?php
		}
		if (($data = mysql_fetch_array($cum_fail)) !== false) {
		?>
		<tr>
			<?php if (is_root()) { ?>
				<td><input type="checkbox" name="d_userfail[]" value="<?= $_GET['user'] ?>" /></td>
			<?php } ?>
			<td colspan="2"><b>All unsuccessful jobs</b> (<?= $data["jobs"] ?>)</td>
			<td align="right"><?= $data["pages"] ?></td>
			<td align="right"><?= sprintf("%.2f", $data["total"]) ?></td>
			<?php if (is_verbose()) { ?>
				<td align="right"><?= sprintf("%.2f", $data["cyan"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["magenta"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["yellow"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["black"]) ?></td>
				<td>&nbsp;</td>
			<?php } ?>
			<td>&nbsp;</td>
		</tr>
		<?php
		}
		if (($data = mysql_fetch_array($cum_all)) !== false) {
		?>
		<tr>
			<?php if (is_root()) { ?>
				<td><input type="checkbox" name="d_user[]" value="<?= $_GET['user'] ?>" /></td>
			<?php } ?>
			<td colspan="2"><b>All jobs</b> (<?= $data["jobs"] ?>)</td>
			<td align="right"><?= $data["pages"] ?></td>
			<td align="right"><?= sprintf("%.2f", $data["total"]) ?></td>
			<?php if (is_verbose()) { ?>
				<td align="right"><?= sprintf("%.2f", $data["cyan"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["magenta"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["yellow"]) ?></td>
				<td align="right"><?= sprintf("%.2f", $data["black"]) ?></td>
				<td>&nbsp;</td>
			<?php } ?>
			<td>&nbsp;</td>
		</tr>
		<?php
		}
		?>
		<?php if (is_root()) { ?>
		<tr>
			<?php if (is_verbose()) { ?>
				<td>&nbsp;</td>
				<td colspan="10"><input type="submit"
					value="Delete selected entries" /></td>
			<?php } else { ?>
				<td>&nbsp;</td>
				<td colspan="5"><input type="submit"
					value="Delete selected entries" /></td>
			<?php } ?>
		</tr>
		<?php } ?>
	</table>
	</form>
	<?php
	helptext();
}

function delete_all()
{
	global $DBLINK;

	$ret = mysql_query("truncate printlog", $DBLINK);
	if ($ret === false)
		echo "<p><b>TRUNCATE query failed:</b><br />",
		     "<b>", mysql_error(), "</b></p>";
}

function delete_users($user, $confirmed = false)
{
	global $DBLINK;

	if ($confirmed === false)
		$confirmed = "";
	else
		$confirmed = "and confirmed=$confirmed";

	for ($i = 0; $i < sizeof($user); ++$i) {
		$query = sprintf("delete from printlog where ".
		         "user='%s' $confirmed", $user[$i]);
		echo "<p>$query</p>";
		$ret = mysql_query($query, $DBLINK);
		if ($ret === false) {
			echo "<p><b>DELETE query failed:</b><br />",
			     htmlspecialchars($query, ENT_NOQUOTES), "<br />",
			     "<b>", htmlspecialchars(mysql_error(), ENT_NOQUOTES),
			     "</b><br />",
			     "Operation aborted.</p>";
			break;
		}
	}
}

function delete_jobs($job)
{
	global $DBLINK;

	for ($i = 0; $i < sizeof($job); ++$i) {
		list($queue, $jid, $user) = explode("/", $job[$i]);
		if ($queue == "" || $jid == "" || $user == "") {
			echo "<p><b>Incomplete decoding of entry \"",
			     $job[$i], "\"</b><br />\n",
			     "Aborting operation.</p>";
			break;
		}
		$query = sprintf("delete from printlog where queue='%s' and ".
		         "jid='%u' and user='%s'", sqlencode($queue),
		         sqlencode($jid), sqlencode($user));
		$ret = mysql_query($query, $DBLINK);
		if ($ret === false) {
			echo "<p><b>DELETE query failed:</b><br />",
			     htmlspecialchars($query, ENT_NOQUOTES), "<br />",
			     "<b>", htmlspecialchars(mysql_error(), ENT_NOQUOTES),
			     "</b><br />",
			     "Operation aborted.</p>";
			break;
		}
	}
}

function process_delete()
{
	if (isset($_POST["d_trunc"])) {
		delete_all();
	} else {
		if (isset($_POST["d_user"]))
			delete_users($_POST["d_user"], false);
		if (isset($_POST["d_usersucc"]))
			delete_users($_POST["d_user"], 1);
		if (isset($_POST["d_userfail"]))
			delete_users($_POST["d_user"], 0);
		if (isset($_POST["d_job"]))
			delete_jobs($_POST["d_job"]);
		return;
	}

	header("HTTP/1.1 302 Found");
	header("Location: ".$_SERVER["REQUEST_URI"]);
	exit();
}

force_https();

$Config = config_read("/usr/src/vitalnix/etc/lpacct.conf");
$DBLINK = mysql_pconnect($Config["sqlhost"], $Config["sqluser"],
          $Config["sqlpw"]);
if ($DBLINK === false)
	die("Could not connect to MYSQL");

mysql_query("set names 'utf8'", $DBLINK);
mysql_select_db($Config["sqldb"], $DBLINK);

if (isset($_GET["logout"]))
	do_logout(false);
if (isset($_POST["username"]))
	do_authenticate_login();
else
	session_start();

?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<link rel="stylesheet" href="/intranet.css" type="text/css" />
<style type="text/css">
i { font-family: serif; }
</style>
</head>
<body>

<div align="center">
<?php
if (is_root()) {
	process_delete();
	if (isset($_GET["user"]) && $_GET["user"] != "")
		user_view($_GET["user"]);
	else
		root_view();
} else if (isset($_SESSION["user"]) && $_SESSION["user"] != "") {
	if (!isset($_GET["user"]))
		user_view($_SESSION["user"]);
	else if (isset($_GET["user"]) && $_GET["user"] == $_SESSION["user"])
		user_view($_GET["user"]);
	else
		login_form();
} else {
	login_form();
}
?>


</div>
</body>
</html>
