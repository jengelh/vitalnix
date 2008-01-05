<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxdbdump&nbsp;-- a database dump program</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxdbdump</b> [<b>--vxdb</b> <i>database</i>]
[<b>-t</b> {<b>ldif</b>|<b>mysql</b>|<b>shadow</b>}] [<b>-u</b>
<i>from</i><b>:</b><i>to</i>] [<b>-w</b>
{<b>passwd</b>,<b>shadow</b>,<b>vxshadow</b>,<b>group</b>}[<b>,</b>...]]</code></p>

<h1>Description</h1>

<p class="block"><i>vxdbdump</i> dumps all users and groups from the VXDB.
Although designed for debugging purposes, this utility can be used as a real
dump utility with which the original database <i>content</i> (see below) can be
restored. You can also use it to convert between databases.</p>

<p class="block"><i>vxdbdump</i> can output the data in a number of formats,
but the output is dependent on the capabilities of <i>vxdbdump</i>, so having
an XYZ back-end module does not imply <i>vxdbdump</i> can produce output
XYZ-style.</p>

<p class="block">Note that <i>vxdbdump</i> does only allow you to reconstruct
the database's content, not the table structure. In case of e.g. a MySQL
database, this means that the table names, column names, column order, indexed
fields, etc. are not stored, but whatever is hardcoded is taken as name/option.
But the <i>content</i> remains the same, so if you plan to restore it using
<i>vxdbdump</i> output, you would [only] have to change (in our MySQL case)
<code>/etc/nss-mysql.conf*</code>.</p>

<p class="block">This "problem" with only retaining database content will not
be fixated, as <i>vxdbdump</i> is not designed to be a restore helper. Use
backup mechanisms provided by the database engine instead (mysqldump, slapcat,
etc.)</p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>--vxdb</b> <i>database</i></code></td>
		<td class="t1">Uses the specified database rather than the
		default one defined in the VXDB configuration file.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-t</b> <i>format</i></td>
		<td class="t2">Output format of the dump. This may either be
		<b>ldif</b>, <b>mysql</b> or <b>shadow</b>. Defaults to
		<b>shadow</b>.</td>
	</tr>
	<tr>
		<td class="t1n"><code><b>-u</b> <i>start</i>:<i>end</i></td>
		<td class="t1">Limit dump to the UID range from <i>start</i> to
		(inclusive) <i>end</i>. This does not limit groups when
		<b>-w group</b> is used.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-w</b> <i>list</i>[<b>,</b>...]</td>
		<td class="t2">Parts of the VXDB to dump, separated by comma.
		Possible parts are <b>passwd</b>, <b>shadow</b>,
		<b>vxshadow</b> and <b>group</b>.</td>
	</tr>
</table>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7)</p>

<?php include_once("Base-footer.php"); ?>
