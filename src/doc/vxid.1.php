<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxid&nbsp;-- print user identity</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxid</b> [<b>--vxdb</b> <i>database</i>]
[<i>user</i>]</code></p>

<h1>Description</h1>

<p class="block"><i>vxid</i> will print basic UID/GID information about the
specified user, who may either be adressed by username or numeric UID.</p>

<p class="block">If no <i>user</i> argument is given, <i>vxid</i> will print
the current UID/GID sets.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t1">Uses the specified database rather than the
		default one defined in the VXDB configuration file.</td>
	</tr>
</table>

<h1>See also</h1>

<p><a href="vxfinger.1.php">vxfinger</a>(1)</p>

<?php include_once("Base-footer.php"); ?>
