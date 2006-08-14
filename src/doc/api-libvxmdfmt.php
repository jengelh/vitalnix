<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p><i>libvxmdfmt</i> (and its frontend <i>mdpwlfmt</i>) is used to beautify
MDSYNC transaction logfiles (as generated by <tt>libvxmdsync</tt> and frontends
like <i>mdsync</i> or <i>xmd</i>) into nice, user-readable documents like HTML,
RTF, Plain Text or others. It takes an input, an output, and optionally a style
template file, as well as a so-called style name which describes what the
beautified file looks like. (Those styles which do not take a style filename
have their output hardcoded in the source code currently.) Please see the index
for detailed descriptions of the available styles.</p>

<h1>Structures</h1>

<p class="block">Only the fields of interest are listed.</p>

<p class="code"><tt>
<b>#</b>include &lt;libvxmdfmt/libvxmdfmt.h&gt;<br /> 
<br />
<b>struct</b> pwlfmt_workspace {<br />
&nbsp; &nbsp; ...<br />
&nbsp; &nbsp; <b>char *</b>style_name;<br />
&nbsp; &nbsp; <b>char *</b>input_file;<br />
&nbsp; &nbsp; <b>char *</b>output_file;<br />
&nbsp; &nbsp; <b>char *</b>template_file;<br />
&nbsp; &nbsp; <b>void (*</b>report<b>)</b>(<b>const struct</b> pwlfmt_workspace <b>*</b>, <b>int</b>, <b>int</b>);<br />
&nbsp; &nbsp; ...<br />
};
</tt></p>

<h1>Function overview</h1>

<p class="code"><tt>
<b>#</b>include &lt;libvxmdfmt/libvxmdfmt.h&gt;<br />
<br />
<b>int</b> pwlfmt_new(<b>struct</b> pwlfmt_workspace <b>*</b>ws);<br />
<b>int</b> pwlfmt_process(<b>struct</b> pwlfmt_workspace <b>*</b>ws);<br />
<b>const char *</b>pwlfmt_strerror(<b>int</b> errnum);
</tt></p>

<p class="code"><tt>
<b>#</b>include &lt;libvxmdfmt/libvxmdfmt.h&gt;<br />
<b>#</b>include &lt;libvxmdfmt/vtable.h&gt;<br />
<br />
<b>struct</b> pwlstyle_vtable <b>*</b>pwlstyles_trav(<b>void **</b>trav);
</tt></p>

<h2>pwlfmt_new</h2>

<p class="block">Initializes the workspace and opens the files and style for
I/O. Returns an AEE code.</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>PWLFMT_ENOSTYLE</tt></td>
    <td class="t1">The specified style was not found</td>
  </tr>
  <tr>
    <td class="t2"><tt>PWLFMT_EREQTPL</tt></td>
    <td class="t2">The style requires a template file, but no file has been
      specified</td>
  </tr>
  <tr>
    <td class="t1"><tt>PWLFMT_EEINPUT</tt></td>
    <td class="t1">Could not open the input file. The exact error (e.g.
      <tt>ENOENT</tt>, etc.) is in <tt>errno</tt>.</td>
  </tr>
  <tr>
    <td class="t2"><tt>PWLFMT_EEOUTPUT</tt></td>
    <td class="t2">Could not open the output file. The exact error (e.g.
      <tt>EPERM</tt>, etc.) is in <tt>errno</tt>.</td>
  </tr>
  <tr>
    <td class="t1"><tt>PWLFMT_EINPUTVER</tt></td>
    <td class="t1">The input file's format is not supported</td>
  </tr>
</table>

<h2>pwlfmt_process</h2>

<p class="block">Starts the process by reading from the input stream, sorting
the data and printing it out to the output stream. Returns an AEE code.</p>

<h2>pwlfmt_strerror</h2>

<p class="block"><tt>pwlfmt_strerror()</tt> returns a string describing the
error <tt>errnum</tt>. If <tt>errnum</tt> is not an error code as returned by
<tt>pwlfmt_new()</tt> or <tt>pwlfmt_process()</tt>, this function will return
<tt>NULL</tt>.</p>

<h2>pwlstyles_trav</h2>

<p class="block">Allows an application to traverse the list of available
styles. <tt>*trav</tt> must be NULL on the first call to
<tt>pwlstyles_trav()</tt>. The function returns a pointer to <tt>struct
pwlstyle_vtable</tt> for each style, which can then be examined for name,
description and requirement of a style. Returns <tt>NULL</tt> when there are no
more styles. Example see below.</p>

<h1>Examples</h1>

<p class="code"><tt>
<b>struct</b> pwlfmt_workspace w <b>=</b> {<br />
&nbsp; &nbsp; .style_name &nbsp;<b>=</b> "pg_html",<br />
&nbsp; &nbsp; .input_file &nbsp;<b>=</b> "synclog-20060220-083226.log",<br />
&nbsp; &nbsp; .output_file <b>=</b> "synclog-20060220.html",<br />
};<br />
pwlfmt_new(<b>&amp;</b>w);
</tt></p>

<p class="code"><tt>
<b>struct</b> pwlstyle_vtable <b>*</b>table;<br />
<b>void *</b>trav <b>=</b> NULL;<br />
<b>while</b>((table <b>=</b> pwlstyles_trav(<b>&amp;</b>trav)) <b>!=</b> NULL)<br />
&nbsp; &nbsp; printf("%s: %s%s\n", table<b>-&gt;</b>name, table<b>-&gt;</b>desc,<br />
&nbsp; &nbsp; &nbsp; table<b>-&gt;</b>require_template <b>?</b> " (requires template)" <b>:</b> "");
</tt></p>

<?php include_once("Base-footer.php"); ?>
