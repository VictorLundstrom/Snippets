using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
public class ShopItemScript : MonoBehaviour
{
    [Header("Upgrade options")]
    [SerializeField] private bool _lightBoots;
    [SerializeField] private bool _heavyBoots;
    [SerializeField] private bool _gripGloves;
    [SerializeField] private bool _healthBoost;

    public Text _nameDisplay;
    public int _cost = 10;
    public Animator itemAnimator;
    [SerializeField] private GameObject iconScreen;
    [SerializeField] private GameObject textScreen;
    private bool isBought = false;
    private float iconValue = 0.5f;
    private float textValue;
 

    private void Awake()
    {
        if (_lightBoots == true)
        {
            _nameDisplay.text = "Light Boots";
        }

        if (_heavyBoots == true)
        {
            _nameDisplay.text = "Heavy Boots";
        }

        if (_gripGloves == true)
        {
            _nameDisplay.text = "Grip Gloves";
        }

        if (_healthBoost == true)
        {
            _nameDisplay.text = "Health Boost";
        }
    }

    private void OnTriggerEnter(Collider other)
    {
        if (!other.transform.root.GetComponent<DontDestroy>()) //If we did NOT collide with a player
            return;


        if (other.transform.root.GetChild(0).GetComponent<PlayerController>().Tokens >= 10 && !isBought)
        {
            if (_lightBoots)
            {
                other.transform.root.GetChild(0).GetComponent<PlayerController>().LightBoots = true;
            }

            if (_heavyBoots)
            {
                other.transform.root.GetChild(0).GetComponent<PlayerController>().HeavyBoots = true;
            }

            if (_gripGloves)
            {
                other.transform.root.GetChild(0).GetComponent<PlayerController>().GripGloves = true;
            }

            if (_healthBoost)
            {
                other.transform.root.GetChild(0).GetComponent<PlayerController>().HealthBoost = true;
            }
            isBought = true;
            BuyPowerup(other.gameObject);
            FMODUnity.RuntimeManager.PlayOneShot("event:/Master/SFX/Pick up", transform.position);
            
        }
    }

    private void BuyPowerup(GameObject playerInstance)
    {
        playerInstance.transform.root.GetChild(0).GetComponent<PlayerController>().Tokens -= _cost;
        Destroy(gameObject);
        itemAnimator.SetTrigger("SelectedItem");
        iconValue = 0f;
        textValue = 0f;
        iconScreen.GetComponent<MeshRenderer>().material.SetFloat("_A", iconValue);
        textScreen.GetComponent<MeshRenderer>().material.SetFloat("_R", textValue);
        textScreen.GetComponent<MeshRenderer>().material.SetFloat("_G", textValue);
        textScreen.GetComponent<MeshRenderer>().material.SetFloat("_B", textValue);
        textScreen.GetComponent<MeshRenderer>().material.SetFloat("_A", textValue);
    }
}
