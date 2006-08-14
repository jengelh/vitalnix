<?php include_once("Base-header.php"); ?>

<h1>Synopsis</h1>

<p class="code"><tt>vxcl_pdbdump <b>[</b>--ldap<b>]</b> <b>[</b>--mysql<b>]</b>
<b>[</b>--shadow<b>]</b></tt></p>

<h1>Description</h1>

<p class="block"><i>pdbdump</i> dumps all users and groups from the PDB.
Although designed for debugging purposes, this utility can be used as a real
dump utility with which the original database <i>content</i> (see below) can be
restored. You can also use it to convert between databases.</p>

<p class="block"><i>pdbdump</i> can output the data in a number of formats, but
the output is dependent on the capabilities of <tt>pdbdump</tt>, so having an
XYZ back-end module does not imply <tt>pdbdump</tt> can produce output
XYZ-style.</p>

<p class="block">Note that <i>pdbdump</i> does only allow you to reconstruct
the database's content, not the table structure. In case of e.g. a MySQL
database, this means that the table names, column names, column order, indexed
fields, etc. are not stored, but whatever is hardcoded is taken as name/option.
But the <i>content</i> remains the same, so if you plan to restore it using
<i>pdbdump</i> output, you would [only] have to change (in our MySQL case)
<tt>/etc/nss-mysql.conf*</tt>.</p>

<p class="block">This "problem" with only retaining database content will not
be fixed, as <i>pdbdump</i> is not designed to be a restore helper. Use backup
mechanisms provided by the database engine instead (<i>mysqldump</i>,
<i>slurpd</i>, etc.)</p>

<?php include_once("Base-footer.php"); ?>
