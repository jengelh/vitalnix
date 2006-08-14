<?php include_once("Base-header.php"); ?>

<h1>Description</h1>

<p class="block"><i>libvxplex</i> is a library to provide a few functions that
deal with the delegation of program flow control. A program may come with
multiple output methods, among which there can be:</p>

<ul>
  <li>NIO - no input/output daemon mode</li>
  <li>CLI - command line interface</li>
  <li>CUI - console user interface (not implemented)</li>
  <li>GUI - graphical user interface</li>
</ul>

<p class="block">Because either output method may require extra libraries, and
because such libraries may not be available, intrusive output methods (like the
GUI, which requires wxGTK) are outsourced to their own submodule. That way, the
main program can be started without a dependency on e.g. wxGTK. This eases
development and distribution of binary packages.</p>

<h1>Interface</h1>

<h2>Include file</h2>

<p class="code"><tt><b>#</b>include &lt;libvxplex/libvxplex.h&gt;</tt></p>

<h2>plex_enter</h2>

<p class="code"><tt><b>int</b> plex_enter(<b>const char *</b>library, <b>const
char *</b>function, <b>int</b> argc, <b>const char **</b>argv, <b>void
*</b>ptr);</tt></p>

<p class="block">The <tt>plex_enter()</tt> function loads the specified library
and calls to <tt>function</tt> in that library passing it the three parameters
<tt>argc</tt>, <tt>argv</tt> and a custom pointer, <tt>ptr</tt>.</p>

<p class="block">On failure, <tt>127</tt> is returned, while on success,
whatever the function in the extra library returns is returned. Note that the
extra function may also return <tt>127</tt>, but the developer using this
function can workaround this if needed without breaking anything.</p>

<h2>plex_select_ui</h2>

<p class="code"><tt><b>unsigned int</b> plex_select_ui(<b>int *</b>argc,
<b>const char ***</b>argv);</tt></p>

<p class="block">This function extracts the <tt>--nio</tt>, <tt>--cli</tt> or
<tt>--gui</tt> arguments from <tt>*argv</tt> and returns the interface the user
selected, if any. All other arguments remain untouched. Possible return values
are:</p>

<table border="1" class="bordered">
  <tr>
    <td class="t1"><tt>PLEXUI_AUTO</tt></td>
    <td class="t1">No argument was given</td>
  </tr>
  <tr>
    <td class="t2"><tt>PLEXUI_NIO</tt></td>
    <td class="t2">The NIO interface was requested</td>
  </tr>
  <tr>
    <td class="t1"><tt>PLEXUI_CLI</tt></td>
    <td class="t1">The CLI interface was requested</td>
  </tr>
  <tr>
    <td class="t2"><tt>PLEXUI_GUI</tt></td>
    <td class="t2">The GUI interface was requested</td>
  </tr>
</table>

<?php include_once("Base-footer.php"); ?>
