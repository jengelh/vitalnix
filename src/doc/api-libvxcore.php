<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block"><i>libvxcore</i> provides functions for the ultimate basic
functions such as module registering and lookup functions for such.</p>

<h1>Function overview</h1>

<p class="code"><code>
<b>#</b>include &lt;vitalnix/libvxcore/loader.h&gt;<br />
<br />
<b>int</b> vxcore_module_register(<b>const char *</b>section, <b>const char *</b>name, <b>const void *</b>ptr);<br />
<b>void</b> vxcore_module_unregister(<b>const char *</b>section, <b>const char *</b>name);<br />
<b>struct</b> HXbtree <b>*</b>vxcore_section_lookup(<b>const char *</b>section);<br />
<b>void *</b>vxcore_module_lookup(<b>const char *</b>section, <b>const char *</b>name);
</code></p>

<h2>vxcore_module_register</h2>

<p class="block">Registers a module in <code>section</code> and stores the
user-defined <code>ptr</code> for it, which can later be retrieved using the
lookup functions described below. Returns an AEE code.</p>

<h2>vxcore_module_unregister</h2>

<p class="block">Unregisters the <code>module</code> in
<code>section</code>.</p>

<h2>vxcore_section_lookup</h2>

<p class="block"><code>vxcore_section_lookup()</code> returns the tree
associated with <code>section</code>, which can be used for traversal and
finding out about registered modules belonging to this section. If the section
does not exist, <code>NULL</code> is returned.</p>

<h2>vxcore_module_lookup</h2>

<p class="block"><code>vxcore_module_lookup</code> returns the user-defined
pointer for <code>module</code> in <code>section</code>. If the section does
not exist or the module is not registered, <code>NULL</code> is returned.</p>

<?php include_once("Base-footer.php"); ?>
