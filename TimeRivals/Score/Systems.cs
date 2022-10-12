using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Systems : MonoBehaviour
{
    [SerializeField] private GameObject _totalScorePrefab;

    private void Awake()
    {
        DontDestroyOnLoad(gameObject);
    }
}
