<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxckuuid&nbsp;-- EDS UUID discrepancy checker</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxckuuid</b> [<b>--vxdb</b> <i>database</i>]
[<b>-T</b> <i>limit</i>] <b>-i</b> <i>file</i> [<b>-t</b>
<i>type</i>]</code></p>

<h1>Description</h1>

<p class="block"><i>vxckuuid</i> will help find minimal differences between EDS
and VXDB users. Such changes are often corrections of user data (i.e. fixing
typos) on the EDS side. However, libvxmdsync (used by <i>vxusersync</i> and
Steelmill) regards every EDS difference as a old/new user (delete/create).
Since it is impossible to say whether an EDS change is just a typo or a new
user, manual user intervention is required, usually by utilizing
<i>vxfixuuid</i> and/or <i>vxusermod</i>, adjusting the VXDB with the new EDS
data, before running <i>vxusersync</i>.</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t1">Uses the specified database rather than the
		default one defined in the VXDB configuration file.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-T</b> <i>limit</i></code></td>
		<td class="t2">Omit displaying comparisons where the computed
		distance is larger than <i>limit</i>. Defaults to 1000.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-i</b> <i>file</i></code></td>
		<td class="t1">External Data Source input file</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-t</b> <i>type</i></code></td>
		<td class="t1">External Data Source type</td>
	</tr>
</table>

<h1>Description, continued</h1>

<p class="block">In External Data Source formats where no external UUID is
provided, Vitalnix has to generate one. This generated UUID is based upon
the user's real name, so if the real name received a typo fix, the UUID
changes and triggers a new-user-discovered event in libvxmdsync. Hence you use
vxfixuuid to not cause this event in the first place.</p>

<p class="block">Note about complexity: vxckuuid will take linear time 
(O(<i>m</i>*<i>n</i>)) to calculate difference between <i>m</i> EDS and
<i>n</i> VXDB users.</p>

<p class="block">vxckuuid will display each EDS-VXDB user pair, sorted by
smallest difference, up to the difference limit given by the <code>-T</code>
option. Output will look like:</p>

<p class="code"><code>
[ &nbsp; 1]: - <i>EDS_Computed_UUID</i> &nbsp;<i>EDS_User_Realname</i><br />
&nbsp;0/1/1: + <i>VXDB_Stored_UUID&nbsp;</i> &nbsp;<i>VXDB_User_Realname</i>
</code></p> 

<p class="block">The number in square brackets gives the total distance. The
other three numbers denote UUID distance, Name distance and weighthing factor.
The exact algorithm can be obtained from the <code>ckuuid.c</code> source
code.</p>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7), <a
href="vxfixuuid.8.php">vxfixuuid</a>(8), <a
href="vxusermod.8.php">vxusermod</a>(8), <a
href="vxusersync.8.php">vxusersync</a>(8)</p>

<?php include_once("Base-footer.php"); ?>
