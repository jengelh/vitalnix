<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The "nss1" driver solely exists for demonstration purposes and
as a code example. Because it uses libc's <tt>getpwent()</tt> and related
functions to lookup users and groups, this module is neither multi-thread-safe
nor multi-use-safe (multiple instances in at least one thread).</p>

<h1>Limitations</h1>

<p class="block">The module does not support adding, modifying or deleting
users and/or groups, and finding the next free UID/GID is not implemented.</p>

<?php include_once("Base-footer.php"); ?>
