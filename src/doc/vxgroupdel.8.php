<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxgroupdel&nbsp;-- Delete a group</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxgroupdel</b> [<b>-F</b>] [<b>--after</b>
<i>command</i>] [<b>--before</b> <i>command</i>] [<b>--vxdb</b>
<i>database</i>] <i>group</i></code></p>

<h1>Description</h1>

<p class="block"><i>vxgroupdel</i> deletes the specified group, but will take
care for whether any use has that group as a primary group set. All options
provided are unique to Vitalnix. The configuration file for <i>vxgroupdel</i>
is <code>/etc/vitalnix/groupdel.conf</code>.</p>

<p class="block">Note that after deletion of a group, there might be objects
left in the filesystem which have the just deleted group's GID as group owner.
One should look after these files and adjust their group membership (if
applicable), to avoid the case that a future group will accidentally get access
because it has the same GID as the previous group.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--after</b> <i>command</i></code></td>
		<td class="t1">Runs the specified command after the group was
		successfully added. If <code>--after</code> is given, but with
		a zero-length command string, the default command in the
		configuration file is not run.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>--before</b>
		<i>command</i></code></td>
		<td class="t2">Runs the specified command before the group is
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
		<td class="t2n"><code><b>-F</b></code></td>
		<td class="t2"><i>vxgroupdel</i> will, by default, not
		delete any groups, of which a user is a primary member of.
		<code>-F</code> forces to delete the group.</td>
	</tr>
</table>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7), <a
href="vxgroupadd.8.php">vxgroupadd</a>(8), <a
href="vxgroupmod.8.php">vxgroupmod</a>(8)</p>

<?php include_once("Base-footer.php"); ?>
