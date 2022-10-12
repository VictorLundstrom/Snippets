using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
public class FillScript : MonoBehaviour
{
    private PlayerMovementBehaviour _playerMovementBehavior;
    public Image FillImage;
    // Start is called before the first frame update
    void Start()
    {
        _playerMovementBehavior = GetComponentInParent<PlayerMovementBehaviour>();
    }

    // Update is called once per frame
    void Update()
    {
        FillImage.fillAmount = _playerMovementBehavior.CurrPickupTime;
    }
}
