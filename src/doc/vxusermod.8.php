<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxusermod&nbsp;-- modify a user account</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxusermod</b> [<b>-L</b>] [<b>--after</b>
<i>command</i>] [<b>--before</b> <i>command</i>] [<b>--vxdb</b>
<i>database</i>] [<b>-A</b> <i>group</i>[<b>,</b>...]] [<b>-G</b>
<i>group</i>[<b>,</b>...]] [<b>-R</b> <i>group</i>[<b>,</b>...]] [<b>-c</b>
<i>comment</i>] [<b>-d</b> <i>directory</i> [<b>-m</b>]] [<b>-e</b>
<i>date</i>] [<b>-f</b> <i>inactive_days</i>] [<b>-g</b> <i>primary_group</i>]
[<b>-l</b> <i>login_name</i>] [<b>-s</b> <i>shell</i>] [<b>-u</b> <i>uid</i>
[<b>-o</b>]] <i>login</i></code></p>

<h1>Description</h1>

<p class="block"><i>usermod</i> updates the user's account with the information
given on the command line. Not specified options will not change the
corresponding account field.</p>

<p class="block">If <code>-I</code> is passed, the user is interactively asked
for what parameters shall be changed, by presenting a default value, which he
may either accept (by leaving the answer empty) or specifying a new value. The
<code>-A</code>, <code>-B</code>, <code>-I</code>, and <code>-M</code> options
are unique to Vitalnix. The configuration file for <code>usermod</code> is
<code>/etc/vitalnix/usermod.conf</code>.</p>

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
		<td class="t1n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t1">Uses the specified database rather than the
		default one listed in the VXDB configuration file.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-A</b>
		<i>group</i>[<b>,</b>...]</code></td>
		<td class="t2">Add the user into the listed secondary
		groups.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-G</b>
		<i>group</i>[<b>,</b>...]</code></td>
		<td class="t1">Set the user's list of secondary groups.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-L</b></code></td>
		<td class="t2">Lock the user's account.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-R</b>
		<i>group</i>[<b>,</b>...]</code></td>
		<td class="t1">Remove the user from the listed secondary
		groups.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-U</b></code></td>
		<td class="t2">Unlock the user's account.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-c</b> <i>comment</i></code></td>
		<td class="t1">Changes the comment (GECOS) field</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-d</b> <i>directory</i></code></td>
		<td class="t2">Changes the home directory. Also see the
		<code>-m</code> option.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-e</b> <i>date</i></code></td>
		<td class="t1">The new date on which the user account will
		expire. (Valid format is YYYY-MM-DD.)</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-f</b> <i>days</i></code></td>
		<td class="t2">The number of days which need to pass after a
		password expiration (which is not <code>-e</code>!) until the
		account is locked down.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-g</b> <i>group</i></code></td>
		<td class="t1">Changes the users initial (primary) group.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-l</b> <i>login</i></code></td>
		<td class="t2">Changes the login name for the user, which must
		be unique.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-m</b></code></td>
		<td class="t1">When <code>-d</code> is given, specifying
		<code>-m</code> will move data from the old home directory into
		the new one, necessarily creating the new directory if it does
		not exist yet.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-o</b></code></td>
		<td class="t2">If a user with the explicit UID given by
		<code>-u</code> already exists, you can override the error
		message with this option, to create non-unique UIDs.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-s</b> <i>shell</i></code></td>
		<td class="t1">Changes the user's default shell.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-u</b> <i>uid</i></code></td>
		<td class="t2">Changes the user's identification number. It
		must be unique, unless <code>-o</code> is given.</td>
	</tr>
</table>

<h1>See also</h1>

<p><a href="vxuseradd.8.php">vxuseradd</a>(8), <a
href="vxuserdel.8.php">vxuserdel</a>(8)</p>

<?php include_once("Base-footer.php"); ?>
