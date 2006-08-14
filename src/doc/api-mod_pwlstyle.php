<?php include_once("Base-header.php"); ?>

<h1>Module definition</h1>

<p class="block">Each style must provide a <tt>struct pwlstyle_vtable
THIS_STYLE</tt> structure that describes the particular style module and its
functions. The codebase of a minimal style could look like:</p>

<p class="code"><tt>
<b>#</b>include &lt;libvxmdfmt/internal.h&gt;<br />
<b>#</b>include &lt;libvxmdfmt/static-build.h&gt;<br />
<b>#</b>include &lt;libvxmdfmt/vtable.h&gt;<br />
<br />
COND_STATIC <b>const struct</b> pwlstyle_vtable THIS_STYLE <b>=</b> {<br />
&nbsp; &nbsp; .name &nbsp; &nbsp; &nbsp;<b>=</b> "mystyle",<br />
&nbsp; &nbsp; .desc &nbsp; &nbsp; &nbsp;<b>=</b> "A dummy style module for testing",<br />
&nbsp; &nbsp; .tbl_entry <b>=</b> mystyle_tbl_entry,<br />
};<br />
<br />
STATIC_REGISTER(mystyle, <b>&amp;</b>THIS_STYLE);
</tt></p>

<p class="block">Unlike <i>libvxpdb</i> which primarily loads submodules on
demand (backend modules can also be compiled in), styles are currently always
linked into their parent library, <i>libvxmdfmt</i>. The <tt>COND_STATIC</tt>
and <tt>STATIC_REGISTER</tt> macros provide the additional helper code, just as
described in the <a href="api-mod_backend.php">backend module API</a>.</p>

<p class="block">All function pointers in the <tt>THIS_STYLE</tt> struct, with
the exception of <tt>-&gt;tbl_entry</tt>, may be <tt>NULL.</tt></p>

<?php include_once("Base-footer.php"); ?>
