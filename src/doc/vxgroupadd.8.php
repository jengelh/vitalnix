<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxgroupadd&nbsp;-- Create a new group</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxgroupadd</b> [<b>--after</b> [<i>command</i>]]
[<b>--before</b> [<i>command</i>]] [<b>--vxdb</b> <i>database</i>] [<b>-g</b>
<i>gid</i> [<b>-o</b>]|<b>-r</b>] <i>group</i></code></p>

<h1>Description</h1>

<p class="block"><i>vxgroupadd</i> will create a new group using the supplied
parameters. The configuration file for <i>vxgroupadd</i> is
<code>/etc/vitalnix/groupadd.conf</code>.</p>

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
		<td class="t2">The numerical group identification number of the
		new group.  The default is to assign the group the next
		greatest GID available. This option overrides
		<code>-r</code>.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-o</b></code></td>
		<td class="t1">If a group with the explicit GID given by
		<code>-g</code> already exists, you can override the error
		message with this option, to create non-unique GIDs.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-r</b></code></td>
		<td class="t2">Create a system group, that is, with a lower GID
		than the hardcoded default (100), or what is specified for
		<code>MIN_GID</code> in the autouid configuration file. The
		<code>-g</code> option overrides this one.</td>
	</tr>
</table>

<h1>See also</h1>

<p><a href="vxgroupmod.8.php">groupmod</a>(8), <a
href="vxgroupdel.8.php">vxgroupdel</a>(8)</p>

<?php include_once("Base-footer.php"); ?>
