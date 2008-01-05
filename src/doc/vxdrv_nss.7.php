<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block">The <i>nss</i> and <i>nss1</i> drivers solely exists for
demonstration purposes and as a code example. Because it uses libc's
<code>getpwent()</code> and related functions to lookup users and groups, this
module is neither multi-thread-safe nor multi-use-safe (multiple instances in
at least one thread).</p>

<h1>Limitations</h1>

<p class="block">The module does not support adding, modifying or deleting
users and/or groups, and finding the next free UID/GID is not implemented.</p>

<h1>See also</h1>

<p><a href="vitalnix.7.php">vitalnix</a>(7)</p>

<?php include_once("Base-footer.php"); ?>
