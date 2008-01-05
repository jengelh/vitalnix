<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxfinger&nbsp;-- user information lookup program</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxfinger</b> [<b>-Igi</b>] [<b>--vxdb</b>
<i>database</i>] <i>substring</i></code></p>

<h1>Description</h1>

<p class="block"><i>vxfinger</i> greps through the VXDB searching for
<i>substring</i> in either the username, numeric UID or real name and displays
matching results. Additionally, it prints information about logged-in status
and mailbox, like the original <code>finger</code>(1) utility.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t1">Uses the specified database rather than the
		default one defined in the VXDB configuration file.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-I</b></code></td>
		<td class="t2">Use case-sensitive matching</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-g</b></code></td>
		<td class="t1">Match the full GECOS field (instead of just the
		first part until the first comma) and also display it in
		full.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-i</b></code></td>
		<td class="t2">Use case-insensitive matching (this is the
		default)</td>
	</tr>
</table>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7), <a
href="vxid.1.php">vxid</a>(1)</p>

<?php include_once("Base-footer.php"); ?>
