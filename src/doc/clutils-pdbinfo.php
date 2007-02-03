<?php include_once("Base-header.php"); ?>

<h1>Usage</h1>

<p class="code"><code>pdbinfo <b>[</b>-O<b>]</b> <b>[</b>-L
<i>dir</i><b>]</b></code></p>

<h1>Description</h1>

<p class="block"><i>pdbinfo</i> will load all backend modules and print their
descriptive name, description, and author. When used with the <code>-O</code>
option, the database(s) are opened and closed too. It is basically just an
information and debugging tool. You can specify additional search directories
for backend modules using the <code>-L</code> option, e.g.:</p>

<p class="code"><code>pdbinfo -L /usr/local/lib -L ~/lib</code></p>

<?php include_once("Base-footer.php"); ?>
