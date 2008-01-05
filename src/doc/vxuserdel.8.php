<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxuserdel&nbsp;-- delete a user account</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxuserdel</b> [<b>-Fr</b>] [<b>--after</b>
<i>command</i>] [<b>--before</b> <i>command</i>] [<b>--vxdb</b>
<i>database</i>] <i>login</i></code></p>

<h1>Description</h1>

<p class="block"><i>vxuserdel</i> deletes the specified user. The configuration
file for <i>vxuserdel</i> is <code>/etc/vitalnix/userdel.conf</code>.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--after</b> <i>command</i></code></td>
		<td class="t1">Runs the specified command after the user was
		successfully added. If <code>--after</code> is given, but with
		a zero-length command string, the default command in the
		configuration file is not run.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>--before</b>
		<i>command</i></code></td>
		<td class="t2">Runs the specified command before the user is
		added. It is run always, even if adding fails. If
		<code>--before</code> is given, but with a zero-length command
		string, the default command in the configuration file is not
		run.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t2">Uses the specified database rather than the
		default one listed in the VXDB configuration file.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-F</b></code></td>
		<td class="t1">Force deletion even if UID is zero or the
		username is <i>root</i>.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-r</b></code></td>
		<td class="t1">Removes the user's home directory including
		files located therein and its mail spool (assuming
		<code>/var/mail/<i>user</i></code>).</td>
	</tr>
</table>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7), <a
href="vxuseradd.8.php">vxuseradd</a>(8), <a
href="vxusermod.8.php">vxusermod</a>(8)</p>

<?php include_once("Base-footer.php"); ?>
