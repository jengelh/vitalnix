<?php include_once("Base-header.php"); ?>

<h1>Usage</h1>

<p class="code"><code>vxdbdump <b>[</b>--ldif<b>]</b> <b>[</b>--mysql<b>]</b>
<b>[</b>--shadow<b>]</b></code></p>

<h1>Description</h1>

<p class="block"><i>vxdbdump</i> dumps all users and groups from the VXDB.
Although designed for debugging purposes, this utility can be used as a real
dump utility with which the original database <i>content</i> (see below) can be
restored. You can also use it to convert between databases.</p>

<p class="block"><i>vxdbdump</i> can output the data in a number of formats,
but the output is dependent on the capabilities of <code>vxdbdump</code>, so
having an XYZ back-end module does not imply <code>vxdbdump</code> can produce
output XYZ-style.</p>

<p class="block">Note that <i>vxdbdump</i> does only allow you to reconstruct
the database's content, not the table structure. In case of e.g. a MySQL
database, this means that the table names, column names, column order, indexed
fields, etc. are not stored, but whatever is hardcoded is taken as name/option.
But the <i>content</i> remains the same, so if you plan to restore it using
<i>vxdbdump</i> output, you would [only] have to change (in our MySQL case)
<code>/etc/nss-mysql.conf*</code>.</p>

<p class="block">This "problem" with only retaining database content will not
be fixed, as <i>vxdbdump</i> is not designed to be a restore helper. Use backup
mechanisms provided by the database engine instead (<i>mysqldump</i>,
<i>slurpd</i>, etc.)</p>

<?php include_once("Base-footer.php"); ?>
