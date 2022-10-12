using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BillboardV2 : MonoBehaviour
{
    public GameObject Target;
    // Start is called before the first frame update
    void Start()
    {
        Target = GameObject.FindGameObjectWithTag("MainCamera");
    }

    // Update is called once per frame
    void Update()
    {
        transform.LookAt(Target.transform.position);
    }
}
