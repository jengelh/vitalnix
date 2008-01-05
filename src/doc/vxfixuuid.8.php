<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxfixuuid&nbsp;-- VX3 UUID correction utility</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxfixuuid</b> [<b>--vxdb</b> <i>database</i>]
[<b>-b</b> <i>date</i>] [<b>-r</b> <i>name</i>] <b>-u</b>
<i>username</i></code></p>

<h1>Description</h1>

<p class="block">The <i>vxfixuuid</i> utility is used to regenerate the
Vitalnix UUID of a user when the data that was used to generate the UUID has
changed, mostly due to typographic fixes. If the UUID was not corrected by the
time the user database is synchronized again to the External Data Source (e.g.
using <i>vxusersync</i>), the user will be deleted and instantly be readded
with a new username, new password and fresh home directory.</p>

<p class="block">When no UUID is provided by the External Data Source, as is
the case with Kolleg/SDF, the UUID is generated from the birthdate and the real
name, so if either change upstream, the UUID needs to be adjusted. If the
External Data Source <i>does</i> already provide a UUID, then there is no
problem.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t1">Uses the specified database rather than the
		default one defined in the VXDB configuration file.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-b</b> <i>date</i></code></td>
		<td class="t2">The new birthdate of the user</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-r</b> <i>name</i></code></td>
		<td class="t1">The new realname of the user</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-u</b> <i>user</i></code></td>
		<td class="t2">The user to modify</td>
	</tr>
</table>

<p class="block">At least one of <code>-b</code> or <code>-r</code> option
needs to be specified.</p>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7), <a
href="vxckuuid.8.php">vxckuuid</a>(8)</p>

<?php include_once("Base-footer.php"); ?>
