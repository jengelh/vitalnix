<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxgroupmod&nbsp;-- Modify a group</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxgroupmod</b> [<b>--after</b> [<i>command</i>]]
[<b>--before</b> [<i>command</i>]] [<b>--vxdb</b> <i>database</i>] [<b>-g</b>
<i>gid</i> [<b>-o</b>]] [<b>-n</b> <i>group_name</i>] <i>group</i></code></p>

<h1>Description</h1>

<p class="block"><i>vxgroupmod</i> updates the group's info with the data given
on the command line. Not specified options will not change the corresponding
fields in the database. The configuration file for <i>vxgroupmod</i> is
<code>/etc/vitalnix/groupmod.conf</code>.</p>

<p class="block">Note that after changing the GID of a group, there might be
objects left in the filesystem which you should pay attention to with respect
to the GID owner. A future group may get the original GID and get accidental
access to files.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--after</b>
		[<i>command</i>]</code></td>
		<td class="t1">Runs the specified command after the group was
		successfully added. If <code>-A</code> is given, but with a
		zero-length command string, the default command in the
		configuration file is not run.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>--before</b>
		[<i>command</i>]</code></td>
		<td class="t2">Runs the specified command before the group is
		added. It is run always, even if adding fails. If
		<code>-B</code> is given, but with a zero-length command
		string, the default command in the configuration file is not
		run.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t1">Uses the specified database rather than the
		default one listed in the VXDB configuration file</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-g</b> <i>gid</i></code></td>
		<td class="t2">Changes the group's identification number. It
		must be unique, unless <code>-o</code> is given.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-n</b> <i>name</i></code></td>
		<td class="t1">Changes the group's name, which must be
		unique.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-o</b></code></td>
		<td class="t2">If a group with the explicit GID given by
		<code>-g</code> already exists, you can override the error
		message with this option, to create non-unique GIDs.</td>
	</tr>
</table>

<h1>See also</h1>

<p><a href="vxgroupadd.8.php">vxgroupadd</a>(8), <a
href="vxgroupdel.8.php">vxgroupdel</a>(8)</p>

<?php include_once("Base-footer.php"); ?>
