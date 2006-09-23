<?php include_once("Base-header.php"); ?>

<h1>Usage</h1>

<p class="code"><tt>pdbinfo <b>[</b>-O<b>]</b> <b>[</b>-L
<i>dir</i><b>]</b></tt></p>

<h1>Description</h1>

<p class="block"><i>pdbinfo</i> will load all backend modules and print their
descriptive name, description, and author. When used with the <tt>-O</tt>
option, the database(s) are opened and closed too. It is basically just an
information and debugging tool. You can specify additional search directories
for backend modules using the <tt>-L</tt> option, e.g.:</p>

<p class="code"><tt>pdbinfo -L /usr/local/lib -L ~/lib</tt></p>

<?php include_once("Base-footer.php"); ?>
