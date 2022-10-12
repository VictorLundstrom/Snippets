using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class PlayerContainerPoints : MonoBehaviour
{
    private GameObject _player;
    private Animator _animator;
    public GameObject Player { get { return _player; } set { _player = value; } }

    public Image Healthbar;

    public GameObject powerUpContainer;

    public TextMeshProUGUI _playerPointsText;

    private void Awake()
    {
        _animator = GetComponent<Animator>();
    }

    public void UpdatePointText()
    {
        _playerPointsText.text = _player.GetComponent<PlayerController>().Points.ToString();
        _animator.SetTrigger("GotKill");
    }
}