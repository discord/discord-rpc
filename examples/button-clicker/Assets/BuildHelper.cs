using UnityEditor;
using System.Diagnostics;
using System.IO;

[InitializeOnLoad]
public class ScriptBatch
{
    static ScriptBatch()
    {
        EnsureDLL();
    }

    public static bool FileExists(string filename)
    {
        return new FileInfo(filename).Exists;
    }
    
    public static void EnsureDLL()
    {
        UnityEngine.Debug.Log("Make sure Discord dll exists");

        string dstDll32 = "Assets/Plugins/x86/discord-rpc.dll";
        string dstDll64 = "Assets/Plugins/x86_64/discord-rpc.dll";

        if (!FileExists(dstDll32) || !FileExists(dstDll64))
        {
            string srcDll32 = "../../builds/install/win64-dynamic/bin/discord-rpc.dll";
            string srcDll64 = "../../builds/install/win64-dynamic/bin/discord-rpc.dll";

            if (!FileExists(srcDll32) || !FileExists(srcDll64))
            {
                UnityEngine.Debug.Log("Try to run build script");
                Process proc = new Process();
                proc.StartInfo.FileName = "python";
                proc.StartInfo.Arguments = "build.py";
                proc.StartInfo.WorkingDirectory = "../..";
                proc.Start();
                proc.WaitForExit();
                if (proc.ExitCode != 0)
                {
                    UnityEngine.Debug.LogError("Build failed");
                    return;
                }
            }

            // make sure the dirs exist
            Directory.CreateDirectory("Assets/Plugins");
            Directory.CreateDirectory("Assets/Plugins/x86");
            Directory.CreateDirectory("Assets/Plugins/x86_64");

            // Copy dlls
            FileUtil.CopyFileOrDirectory("../../builds/install/win64-dynamic/bin/discord-rpc.dll", dstDll64);
            FileUtil.CopyFileOrDirectory("../../builds/install/win32-dynamic/bin/discord-rpc.dll", dstDll32);
        }
    }
}