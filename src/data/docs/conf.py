# 
# Docs/conf.py
#
import os
import sys

# --------------------------------------------
# Set up
#
def read_version() -> str:
    """ 
    Get package version from version.txt file
    """
    file = '../../version.txt'
    if os.path.exists(file):
        with open(file, 'r') as fob:
            proj_vers = fob.readlines()[0]
    else:
        proj_vers = '0.1.0-unknown'
    return proj_vers

docs_root = os.path.dirname(os.path.abspath(__file__))
#abs_lib_path = os.path.abspath(os.path.join(docs_root, 'lib'))

# --------------------------------------------
# project
# 
project = "cidrtools"
author = 'Gene C'

release = read_version()

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.imgconverter',
    "hawkmoth",                  # Core Hawkmoth C-Autodoc engine
]

primary_domain = 'c'
hawkmoth_clang_c = [
    "-std=c23",
    "-Ilib",
]

# Tell Sphinx's code highlighter not to emit visible/literal unicode whitespace symbols
# sphinx = baseline, tango = corp color, friendly - brighter, colorful - more so
# Options: sphinx, friendly, tango
# pygments_style = 'sphinx'
latex_engine = 'xelatex'
latex_use_xindy = True

latex_elements = {
    'papersize': 'letterpaper',
    'pointsize': '11pt',

    'fvset': r'\fvset{fontsize=\scriptsize}',

    'fontpkg': r'''
        \usepackage{fontspec}

        \setmainfont{Source Sans 3}[Ligatures=TeX]
        \setsansfont{Source Sans 3}[Ligatures=TeX]
        \setmonofont{Source Code Pro}
    ''',

    'preamble': r'''
        \usepackage{parskip}
        % \usepackage{needspace}

        %
        % Fix the 11pt headheight layout warnings
        %
        \setlength{\headheight}{14pt}
        \addtolength{\topmargin}{-2pt}

        \usepackage{enumitem}
        \setlist[itemize]{
            noitemsep,
            topsep=6pt,
            parsep=0pt,
            partopsep=0pt,
            after=\vspace{0pt}
        }
        \setlist[enumerate]{
            noitemsep,
            topsep=6pt,
            parsep=0pt,
            partopsep=0pt,
            after=\vspace{0pt}
        }

        \usepackage{newunicodechar}
        \newunicodechar{␣}{\textvisiblespace}
        \tracinglostchars=0

        %\makeatletter
        %\renewcommand{\subsection}[1]{\par\bigskip\needspace{14\baselineskip}\textbf{#1}}
        %%\renewcommand{\subsection}{\par\bigskip\needspace{14\baselineskip}}
        %\makeatother
        ''',
}

# Grouping the document tree into a single LaTeX document manual volume.
# Tuple structure: (source start file, target name, title, author, documentclass)
latex_documents = [
    (
        'index',
        'cidrtools.tex',
        'cidrtools API Reference',
        'Gene C',
        'manual'
    ),
]

html_theme = 'sphinx_rtd_theme'  # Works exactly the same if using 'furo' or 'alabaster'
html_static_path = ['_static']
html_css_files = [ 'custom.css',]

