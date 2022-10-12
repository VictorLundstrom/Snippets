using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponCrossbow : WeaponBaseClass
{
    [SerializeField] private float _selfKnockbackForceMultiplier;
    public float SelfKnockbackForceMultiplier { get { return _selfKnockbackForceMultiplier; } }

    [SerializeField] private GameObject _arrow;
    [SerializeField] private int _arrowSpeed;
    [SerializeField] private Transform _arrowSpawnPoint;

    [SerializeField] private MeshFilter _crossbowMeshFilter;
    [SerializeField] private Mesh CrossbowDefault;
    [SerializeField] private Mesh _crossbowFired;

    protected override void Awake()
    {
        base.Awake();
    }

    public void Fire()
    {
        GameObject arrow = Instantiate(_arrow, _arrowSpawnPoint.position, _arrowSpawnPoint.rotation);
        arrow.GetComponent<Bullet>().PlayerID = playerID;
        arrow.GetComponent<Rigidbody>().AddForce(transform.forward * _arrowSpeed, ForceMode.Impulse);
        _crossbowMeshFilter.mesh = _crossbowFired; //Swap mesh filter to crossbowFired

        CurrDurability -= _DurabilityCost;

        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Bow/Bow string release", transform.position);
        FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Bow/Arrow Swoosh", transform.position);

        if (CurrDurability <= 0)
        {
            ShouldDestroy = true;
        }
    }

    public void ResetMeshDefault() //Swap mesh filter back to default
    {
        _crossbowMeshFilter.mesh = CrossbowDefault;
    }

    public override void ResetVariables()
    {
    }

}
