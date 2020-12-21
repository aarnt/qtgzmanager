<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="gl">
<context>
    <name>PasswordDialog</name>
    <message>
        <location filename="../passworddialog.ui" line="6"/>
        <source>LXQt sudo</source>
        <translation>LXQt sudo</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="42"/>
        <source>Copy command to clipboard</source>
        <translation>Copiar a orde no portapapeis</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="45"/>
        <source>&amp;Copy</source>
        <translation>&amp;Copiar</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="83"/>
        <source>The requested action needs administrative privileges.&lt;br&gt;Please enter your password.</source>
        <translation>A acción solicitada necesita privilexios administrativos.&lt;br&gt;Introduza o seu contrasinal.</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="106"/>
        <source>LXQt sudo backend</source>
        <translation>Infraestrutura do LXQt sudo</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="109"/>
        <source>A program LXQt sudo calls in background to elevate privileges.</source>
        <translation>Un programa LXQt sudo chama en segundo plano para elevar os privilexios.</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="119"/>
        <source>Command:</source>
        <translation>Orde:</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="126"/>
        <source>Password:</source>
        <translation>Contrasinal:</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="133"/>
        <source>Enter password</source>
        <translation>Introduza o contrasinal</translation>
    </message>
    <message>
        <location filename="../passworddialog.cpp" line="60"/>
        <source>Attempt #%1</source>
        <translation>Intento núm. %1</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../sudo.cpp" line="75"/>
        <source>Usage: %1 option [command [arguments...]]

GUI frontend for %2/%3

Arguments:
  option:
    -h|--help      Print this help.
    -v|--version   Print version information.
    -s|--su        Use %3(1) as backend.
    -d|--sudo      Use %2(8) as backend.
  command          Command to run.
  arguments        Optional arguments for command.

</source>
        <translation>Uso: %1 opción [orde [argumentos...]]

Interface gráfica para %2/%3

Argumentos:
  opción:
    -h|--help       Amosa esta axuda.
    -v|--version    Amosa a información da versión.
    -s|--su         Use %3(1) como infraestrutura.
    -d|--sudo       Use %2(8)  como infraestrutura.
  orde              Orde a executar.
  argumentos        Argumentos opcionais para a orde.

</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="92"/>
        <source>%1 version %2
</source>
        <translation>%1 versión %2
</translation>
    </message>
</context>
<context>
    <name>Sudo</name>
    <message>
        <location filename="../sudo.cpp" line="195"/>
        <source>%1: no command to run provided!</source>
        <translation>%1: non forneceu ningunha orde!</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="202"/>
        <source>%1: no backend chosen!</source>
        <translation>%1: non se escolleu ningunha infraestrutura!</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="219"/>
        <source>Syscall error, failed to fork: %1</source>
        <translation>Produciuse un erro de chamada ao sistema, non foi posíbel crear o subproceso: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="246"/>
        <source>unset</source>
        <extracomment>shouldn&apos;t be actually used but keep as short as possible in translations just in case.</extracomment>
        <translation>sen fixar</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="295"/>
        <source>%1: Detected attempt to inject privileged command via LC_ALL env(%2). Exiting!
</source>
        <translation>%1: Detectouse un intento de inxectar ordes con privilexios a través de LC_ALL env(%2). Saíndo!
</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="337"/>
        <source>Syscall error, failed to bring pty to non-block mode: %1</source>
        <translation>Produciuse un erro de chamada ao sistema, non foi posíbel poñer «pty» en modo desbloqueado: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="345"/>
        <source>Syscall error, failed to fdopen pty: %1</source>
        <translation>Produciuse un erro de chamada ao sistema, non foi posíbel aplicar «fdopen» a «pty»: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="314"/>
        <source>%1: Failed to exec &apos;%2&apos;: %3
</source>
        <translation>%1: Produciuse un fallo ao executar «%2»: %3
</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="376"/>
        <source>Child &apos;%1&apos; process failed!
%2</source>
        <translation>Produciuse un fallo no proceso fillo «%1»!
%2</translation>
    </message>
</context>
</TS>
