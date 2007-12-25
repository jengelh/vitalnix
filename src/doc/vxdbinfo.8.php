<?php include_once("Base-header.php"); ?>

<h1>Name</h1>

<p>vxdbinfo&nbsp;-- a database module information program</p>

<h1>Syntax</h1>

<p class="code"><code><b>vxdbinfo</b> [<b>-O</b>] [<b>-L</b>
<i>dir</i>]</code></p>

<h1>Description</h1>

<p class="block"><i>vxdbinfo</i> will load all backend modules and print their
descriptive name, description, and author. When used with the <code>-O</code>
option, the database(s) are opened (in read-only mode) and closed too. It is
basically just an information and debugging tool. You can specify additional
search directories for backend modules using the <code>-L</code> option,
e.g.:</p>

<p class="code"><code>vxdbinfo -L /usr/local/lib -L ~/lib</code></p>

<h1>Options</h1>

<table border="1">
	<tr>
		<td class="t1n"><code><b>-L</b> <i>dir</i></code></td>
		<td class="t1">Search this directory for Vitalnix database
		modules too.</td>
	</tr>
	<tr>
		<td class="t2n"><code><b>-O</b></code></td>
		<td class="t2">Do not just load the database modules, but also
		call the <code>vxdb_open()</code> function.</td>
	</tr>
</table>

<?php include_once("Base-footer.php"); ?>
