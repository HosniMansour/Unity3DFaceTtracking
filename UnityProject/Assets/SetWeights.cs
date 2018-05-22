using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System;
using System.Text;

public class SetWeights : MonoBehaviour
{
    int blendShapeCount;
    SkinnedMeshRenderer skinnedMeshRenderer;
    Mesh skinnedMesh;
    public Transform head;
    public Transform lefteye;
    public Transform righteye;
    public Transform ltarget;
    public Transform rtarget;
    Vector3 hr;

    // Open Camera and getdetection_success fns
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "getdetection_success", CallingConvention = CallingConvention.Cdecl)]
    static extern int getdetection_success();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "main", CallingConvention = CallingConvention.StdCall)]
    static extern int main();

    // Get Head pose [it could be replaced by one function by returning an array of doubles like in get XY]
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_pose1", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_pose1();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_pose2", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_pose2();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_pose3", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_pose3();

    // Get eyes gaze [it could be replaced by one function by returning an array of doubles like in get XY]
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze1", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze1();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze2", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze2();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze3", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze3();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze4", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze4();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze5", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze5();
    [DllImport("FaceLandmarkVid.dll", EntryPoint = "get_gaze6", CallingConvention = CallingConvention.Cdecl)]
    static extern float get_gaze6();

    // Get X Y Face Position (double array) | 2D landmark [x1;x2;...xn;y1;y2...yn] 
    [DllImport("FaceLandmarkVid.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int getXY(out IntPtr pArrayOfDouble);
    double[] ArrayOfDouble = new double[140];

    // Thread to Open Camera 
    System.Threading.Thread newThread = new System.Threading.Thread(AMethod);
    private static void AMethod()
    {
        main();
    }

    // Use this for initialization
    void Awake()
    {
        skinnedMeshRenderer = GetComponent<SkinnedMeshRenderer>();
        skinnedMesh = GetComponent<SkinnedMeshRenderer>().sharedMesh;
    }

    // Use this for initialization
    void Start()
    {
        newThread.Start();
        InvokeRepeating("setBlendShapes", 0, 0.033f);
    }

    // Update is called once per frame
    void Update()
    {
    }

    //in succession
    private void setBlendShapes()
    {

        double[] ArrayOfDouble = new double[140];
        double[] skinn = new double[50] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        
        if (getdetection_success() == 1)
        {
            // Mouth & Eye Brow Animation 

            // ArrayOfDouble de 0 --> 135 !=0 | 136 --> 140 = 0

            IntPtr p2DDoubleArray = IntPtr.Zero;
            getXY(out p2DDoubleArray);
            Marshal.Copy(p2DDoubleArray, ArrayOfDouble, 0, 140);

            skinn[35] = Math.Abs(ArrayOfDouble[51]- ArrayOfDouble[57])*10;


            /*
            Debug.Log(" 0 : " + ArrayOfDouble[0]);
            Debug.Log(" 100 : " + ArrayOfDouble[100]);
            Debug.Log(" 135 : " + ArrayOfDouble[135]);
            */

            //--> this will give exception don't try it 
            //Debug.Log(" 136 : " + ArrayOfDouble[136]);

            // We have 68 ponint of X and 68 point of X but blendShapeCount = 50
            // do some math here to get 50 point 

            /**
             * 
             *   MATHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
             *
             * 
            */

            blendShapeCount = skinnedMesh.blendShapeCount;
            for (int i = 0; i < blendShapeCount; i++)
             {
                 skinnedMeshRenderer.SetBlendShapeWeight(i, (float)skinn[i]);
             }

            // Face Pose Animation

            hr = new Vector3(Mathf.Rad2Deg * get_pose1(), Mathf.Rad2Deg * get_pose2(), Mathf.Rad2Deg * get_pose3());
        }
        else
        {
            hr = new Vector3(0, 0, 0);
        }
        head.rotation = Quaternion.Euler(hr);

        // Eye Look At Animation

        ltarget.localPosition = new Vector3(-1 * (get_gaze1() - 0.13f), get_gaze2() - 0.1f, get_gaze3() * -1);
        rtarget.localPosition = new Vector3(-1 * (get_gaze4() + 0.13f), get_gaze5() - 0.1f, get_gaze6() * -1);
        lefteye.LookAt(ltarget);
        righteye.LookAt(rtarget);

    }

    // Close thread (No effect :v !)
    private void OnApplicationQuit()
    {
        newThread.Abort();
    }
}
