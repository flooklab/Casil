# Contributing

In the long term (i.e. not really at this point already...), contributions (bug reports, source code, etcetera...)
are principally welcome, but you must read and adhere to these contributing terms as listed in the following before
doing so. Also, it is probably a good idea not to contribute unless you have actually used *basil* before.  

Contributions are naturally organized through GitHub's `Issues` and `Pull requests` functionality.
If you want to contribute source code yourself, use the common procedure: fork this repository,
create a (feature-)branch, commit there and open a pull request here.

## Licensing and Copyright

GitHub's own terms should be clear enough on this, but let's reiterate some essential licensing regulations here:
By contributing to Casil you agree that your contributions will be licensed under Casil's [license](LICENSE),
which is the [GNU AGPL 3.0](https://www.gnu.org/licenses/agpl-3.0) license. The contributions must, of course,
be your personal work, be compatible with that license, not violate others' copyright, etcetera.  

For simplicity, your copyright is going to be handled in the following way:  
If a *change* to a file is *not actually copyrightable* (i.e. trivial changes
such as mere formatting, typo corrections, ...) then do nothing, otherwise:
- Add your name to the [CONTRIBUTORS](CONTRIBUTORS) file.
- If the modified files (with copyrightable changes) don't already contain the full  
  `Copyright (C) @YEARS@ M. Frohne and contributors`  
  line in the license notice, append the ` and contributors` part.
- If applicable, include the current year in the affected notices' `@YEARS@` ranges.
- For *new* files, copy the license notice from another file and simply replace its copyright line by  
  `Copyright (C) @CURRENT_YEAR@ Casil contributors`
- Note: Handling copyright information for adapted *basil* code is slightly more complex,
  see [further below](#user-content-adapting-basil-code).

## AI Policy

Please do not include any work from AI in any of your contributions:  

It is expressly forbidden to contribute to Casil any content that has been created with the assistance
of Natural Language Processing artificial intelligence tools. This motion can be revisited, should a
case been made over such a tool that does not pose copyright, ethical and quality concerns.

### Rationale

1. Copyright concerns:  
   At this point, the regulations concerning copyright of generated contents are still emerging
   worldwide. Using such material could pose a danger of copyright violations, but it could
   also weaken Casil claims to copyright and void the guarantees given by copyleft licensing.

2. Quality concerns:  
   Popular LLMs are really great at generating plausibly looking, but meaningless content. They are capable of
   providing good assistance if you are careful enough, but we can't really rely on that. At this point, they
   pose both the risk of lowering the quality of the Casil project, and of requiring an unfair human effort
   from developers and users to review contributions and detect the mistakes resulting from the use of AI.

3. Ethical concerns:  
   The business side of the AI boom is creating serious ethical concerns. Among them:
   - Commercial AI projects are frequently indulging in blatant copyright violations to train their models.
   - Their operations are causing concerns about the huge use of energy and water.
   - The advertising and use of AI models has caused a significant harm to employees and reduction of service quality.
   - LLMs have been empowering all kinds of spam and scam efforts.

### Attribution

The above **AI policy** was adapted from the [Gentoo wiki](https://wiki.gentoo.org/index.php?oldid=1309111),  
Copyright (C) 2024 Gentoo Authors.  
That policy is licensed under the [CC BY-SA 4.0 License](https://creativecommons.org/licenses/by-sa/4.0/).

## Specific Instructions for Contributions

Here follow some more specific rules/guidelines. Note that those are not necessarily
very thorough or complete, though, as your contributions will get reviewed anyway.

### Type/Scope of Contributions

Because Casil is supposed to be(come) a reimplementation of *basil* but currently is not anywhere close to that,
the primary focus of development should really just be on implementing more basil functionality.
Most importantly, this comprises the adaption of more basil *driver* components (see below).  

Other things (except bug fixes, maybe general code quality, simulation tests for added register drivers, ...)
have *no* priority at the moment, especially not new features that are not part of basil.

### Writing new Driver/Interface/Register Components

For adding components (concentrate on drivers; *register* drivers in particular!), choose one component from *basil* and adapt the logic
accordingly. For consistence (and convenience), please start out with the existing *dummy* component files matching the appropriate
base class for the new component, which you can copy, rename and then adjust to and fill with code adapted from the basil component.  

For example, the `GPIO` driver is a `RegisterDriver` (base class), whose associated
dummy component is the `DummyRegisterDriver`, which means that you would copy and modify
[dummyregisterdriver.h](casil/HL/Muxed/dummyregisterdriver.h), [dummyregisterdriver.cpp](casil/HL/Muxed/dummyregisterdriver.cpp)
and [dummyregisterdriver_py.cpp](pycasil/HL/Muxed/dummyregisterdriver_py.cpp). The new files must be included in the
[SourceFiles.cmake](SourceFiles.cmake) file: insert the header file path into `HEADER_FILE_NAMES` and the source file path
into `SOURCE_FILE_NAMES`, leaving out the `.cpp` suffix as the binding file name with `_py.cpp` will be auto-generated from that.
Additionally, the new function signature from the new/modified binding file must be added to the main binding file for the
respective layer, e.g. [hl.cpp](pycasil/HL/hl.cpp) for the hardware layer (as in this example case with the `GPIO` driver).  

Note: While, in general, you should aim for high compatibility, you may deviate a bit from the basil reference where appropriate;
some publicly exposed function/interface might not be used anymore at all by any relevant project, some of the internal logic
might be broken, some useful functionality might be incomplete or missing altogether, etcetera.  

### Adapting *basil* Code

When directly adapting basil code into Casil code, the original copyright and license must be retained.
To comply with this for files that closely reproduce such adapted basil logic, copy the additionally required notice from
a file that already contains it and insert it right below Casil's own notice (see e.g. [sitcp.h](casil/TL/Muxed/sitcp.h)).
For simplicity, extend the respective `@YEARS@` range in the line  
`Copyright (C) @YEARS@ SiLab, Institute of Physics, University of Bonn`  
to the latest modification year of the specific basil file(s) that you used as reference.

### Code Style

I don't want to list exhaustive code style instructions here, simply adhere to the code style
that you find in the existing code! There is one very specific instruction, though: Please limit
the line lengths to maximally 143. This is an arbitrary choice, but it is what it is (a mandatory limit).
