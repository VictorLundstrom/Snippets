using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UpdateMainMenuPanel : MonoBehaviour
{
    [SerializeField] private Material _holoGramMat;
    [SerializeField] private GameObject _portal;
    string _startGame;
    string _1p;
    string _2p;
    string _3p;

    private void Awake()
    {
        _startGame = _holoGramMat.shader.GetPropertyName(2);
        _1p = _holoGramMat.shader.GetPropertyName(3);
        _2p = _holoGramMat.shader.GetPropertyName(4);
        _3p = _holoGramMat.shader.GetPropertyName(5);

        //Default values need to be set because they change permanently even after exiting Playmode
        _holoGramMat.SetFloat(_startGame, 1);
        _holoGramMat.SetFloat(_1p, 0);
        _holoGramMat.SetFloat(_2p, 0);
        _holoGramMat.SetFloat(_3p, 0);
    }



    void Update()
    {
        switch (_portal.GetComponent<Portal>().PlayerIDs.Count)
        {
            case 0:
                _holoGramMat.SetFloat(_startGame, 1f);
                _holoGramMat.SetFloat(_1p, 0f);
                _holoGramMat.SetFloat(_2p, 0f);
                _holoGramMat.SetFloat(_3p, 0f);
                break;
            case 1:
                _holoGramMat.SetFloat(_startGame, 0f);
                _holoGramMat.SetFloat(_1p, 0.3f);
                _holoGramMat.SetFloat(_2p, 0f);
                _holoGramMat.SetFloat(_3p, 0f);
                break;
            case 2: //2 Players
                _holoGramMat.SetFloat(_startGame, 0f);
                _holoGramMat.SetFloat(_1p, 0f);
                _holoGramMat.SetFloat(_2p, 4f);
                _holoGramMat.SetFloat(_3p, 0f);
                break;
            case 3: //3 Players
                _holoGramMat.SetFloat(_startGame, 0f);
                _holoGramMat.SetFloat(_1p, 0f);
                _holoGramMat.SetFloat(_2p, 0f);
                _holoGramMat.SetFloat(_3p, 1f);
                break;

            default:
                return;
        }
    }
    
}
