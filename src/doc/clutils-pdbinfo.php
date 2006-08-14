<?php include_once("Base-header.php"); ?>

<h1>Synopsis</h1>

<p class="code"><tt>vxcl_pdbinfo <b>[</b>-O<b>]</b> <b>[</b>-L
DIR<b>]</b></tt></p>

<h1>Description</h1>

<p class="block"><i>pdbinfo</i> will load all backend modules and print their
descriptive name, description, and author. When used with the <tt>-O</tt>
option, the database(s) are opened and closed too. It is basically just an
information and debugging tool. You can specify additional search directories
for backend modules using the <tt>-L</tt> option, e.g.:</p>

<p class="code"><tt>vxcl_pdbinfo -L /usr/local/lib -L ~/lib</tt></p>

<?php include_once("Base-footer.php"); ?>
